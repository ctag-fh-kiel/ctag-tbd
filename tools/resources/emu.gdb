target remote :1234
set remote hardware-watchpoint-limit 2
maintenance flush register-cache
thb app_main
tui enable
c