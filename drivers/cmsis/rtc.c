// SPDX-FileCopyrightText: 2026 Pablo Palazon
// SPDX-License-Identifier: BSD-3-Clause

#include "rtc.h"

#define RTC_LSE_PRESCALER 0x7FFFu
#define RTC_WAIT_LIMIT 16000000u

// Poll a hardware flag with a bounded wait so a clock failure cannot hang forever.
static bool rtc_wait_for_set(volatile uint32_t *reg, uint32_t mask) {
    for (uint32_t remaining = RTC_WAIT_LIMIT; remaining > 0u; --remaining) {
        if ((*reg & mask) != 0u) {
            return true;
        }
    }

    return false;
}

// Force the APB interface to copy the latest RTC register values from its clock domain.
static bool rtc_wait_registers_synchronized(void) {
    RTC->CRL &= ~RTC_CRL_RSF;
    return rtc_wait_for_set(&RTC->CRL, RTC_CRL_RSF);
}

// Wait until the RTC finishes propagating the previous write to its clock domain.
static bool rtc_wait_write_finished(void) {
    return rtc_wait_for_set(&RTC->CRL, RTC_CRL_RTOFF);
}

// Allow writes to the backup domain, which contains the RTC and backup registers.
static void rtc_enable_backup_writes(void) {
    PWR->CR |= PWR_CR_DBP;
}

// Protect the backup domain again after configuration is complete.
static void rtc_disable_backup_writes(void) {
    PWR->CR &= ~PWR_CR_DBP;
}

// Divide the 32.768 kHz LSE clock into one RTC counter increment per second.
static bool rtc_write_prescaler_1hz(void) {
    if (!rtc_wait_write_finished()) {
        return false;
    }

    RTC->CRL |= RTC_CRL_CNF;
    RTC->PRLH = 0u;
    RTC->PRLL = RTC_LSE_PRESCALER;
    RTC->CRL &= ~RTC_CRL_CNF;

    return rtc_wait_write_finished();
}

bool rtc_init(void) {
    bool configure_prescaler = false;
    const uint32_t clock_source = RCC->BDCR & RCC_BDCR_RTCSEL;

    // Enable the APB interfaces required to access power and backup-domain registers.
    RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
    rtc_enable_backup_writes();

    if (clock_source != RCC_BDCR_RTCSEL_LSE) {
        // Reset retained RTC state because RTCSEL cannot change while the domain is configured.
        RCC->BDCR |= RCC_BDCR_BDRST;
        RCC->BDCR &= ~RCC_BDCR_BDRST;
        configure_prescaler = true;
    } else if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0u) {
        configure_prescaler = true;
    }

    // Start the external 32.768 kHz oscillator before selecting it as the RTC source.
    RCC->BDCR |= RCC_BDCR_LSEON;
    if (!rtc_wait_for_set(&RCC->BDCR, RCC_BDCR_LSERDY)) {
        rtc_disable_backup_writes();
        return false;
    }

    // Keep an existing LSE selection unchanged so the battery-backed counter is preserved.
    if (clock_source != RCC_BDCR_RTCSEL_LSE) {
        RCC->BDCR = (RCC->BDCR & ~RCC_BDCR_RTCSEL) | RCC_BDCR_RTCSEL_LSE;
    }
    RCC->BDCR |= RCC_BDCR_RTCEN;

    // Synchronize the APB-visible registers after enabling the RTC clock.
    if (!rtc_wait_registers_synchronized()) {
        rtc_disable_backup_writes();
        return false;
    }

    // Repair a retained prescaler that was configured by older firmware.
    const uint32_t prescaler = ((RTC->PRLH & RTC_PRLH_PRL) << 16u) | (RTC->PRLL & RTC_PRLL_PRL);
    if (prescaler != RTC_LSE_PRESCALER) {
        configure_prescaler = true;
    }

    // Configure one-second ticks for a new, disabled, or incorrectly configured RTC.
    if (configure_prescaler && !rtc_write_prescaler_1hz()) {
        rtc_disable_backup_writes();
        return false;
    }

    rtc_disable_backup_writes();
    return true;
}

bool rtc_set_prescaler_1hz(void) {
    // Temporarily unlock the backup domain for the protected prescaler write.
    rtc_enable_backup_writes();
    const bool success = rtc_write_prescaler_1hz();
    rtc_disable_backup_writes();
    return success;
}

bool rtc_set_second_interrupt(void) {
    // Temporarily unlock the backup domain for the protected control register
    rtc_enable_backup_writes();

    if (!rtc_wait_write_finished()) {
        rtc_disable_backup_writes();
        return false;
    }

    // Cleaning a pending interruption
    RTC->CRL &= ~(RTC_CRL_SECF);

    // Activating Second Interrupt
    RTC->CRH |= RTC_CRH_SECIE;

    const bool success = rtc_wait_write_finished();
    rtc_disable_backup_writes();

    // Unmask the IRQ only after the RTC register update has completed.
    if (success) {
        NVIC_ClearPendingIRQ(RTC_IRQn);
        NVIC_EnableIRQ(RTC_IRQn);
    }

    return success;
}

bool rtc_set_epoch_time(uint32_t epoch_time_sec) {
    // Temporarily unlock the backup domain for the protected counter write.
    rtc_enable_backup_writes();

    if (!rtc_wait_write_finished()) {
        rtc_disable_backup_writes();
        return false;
    }

    RTC->CRL |= RTC_CRL_CNF;
    RTC->CNTH = epoch_time_sec >> 16u;
    RTC->CNTL = epoch_time_sec & 0xFFFFu;
    RTC->CRL &= ~RTC_CRL_CNF;

    const bool success = rtc_wait_write_finished();
    rtc_disable_backup_writes();
    return success;
}

void rtc_clear_second_flag(void) {
    rtc_wait_write_finished();
    rtc_enable_backup_writes();
    RTC->CRL &= ~RTC_CRL_SECF;
    rtc_disable_backup_writes();
}

uint32_t rtc_get_epoch_time(void) {
    uint32_t high_before;
    uint32_t high_after;
    uint32_t low;

    // Repeat the split-register read if the high half changes while reading the low half.
    do {
        high_before = RTC->CNTH & RTC_CNTH_RTC_CNT;
        low = RTC->CNTL & RTC_CNTL_RTC_CNT;
        high_after = RTC->CNTH & RTC_CNTH_RTC_CNT;
    } while (high_before != high_after);

    return (high_after << 16u) | low;
}
