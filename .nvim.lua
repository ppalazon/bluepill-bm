local dap = require("dap")
local root = vim.fs.root(0, { ".git" }) or vim.fn.getcwd()

local function firmware_elf()
  local source = vim.api.nvim_buf_get_name(0)
  local app = source:match("/apps/([^/]+)/")

  if not app then
    app = vim.fn.input("Application: ", "cmsis-adc-temp-ntc")
  end

  local elf = root .. "/build/bluepill-" .. app .. ".elf"

  if vim.fn.filereadable(elf) == 0 then
    error("Firmware is not built: " .. elf)
  end

  return elf
end

dap.adapters.gdb = {
  type = "executable",
  command = "arm-none-eabi-gdb",
  args = {
    "--quiet",
    "--interpreter=dap",
  },
}

dap.configurations.c = {
  {
    name = "Attach to STM32 through OpenOCD",
    type = "gdb",
    request = "attach",
    program = firmware_elf,
    cwd = root,
    target = "localhost:3333",
  },
}
