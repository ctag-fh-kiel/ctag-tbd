## Ableton Link support for TBD

- Can be disabled in menuconfig TBD options
- Debug means a small thread runs which logs the link status to esp console
- If disabled
  - link is not linked, reducing fw size
  - Api returns in data structure linkActive = false