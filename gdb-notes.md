**Prerequisites**

* Make sure to install the `gdb` package
* Make sure to compile with `-g -gdb3` and `-O0`


**Starting**

Load the binary with
`gdb ./hellband`

**Setting breakpoints and watches**

* `break <filename>:<line>`
* `watch <variable>` (breaks on write access)
* `rwatch <variable>` (breaks on read access)
* `awatch <variable>` (breaks on read and write access)

**Querying variables**

* `info variables` to list "All global and static variable names".
* `info locals` to list "Local variables of current stack frame" (names and values)
* `info args` to list "Arguments of the current stack frame" (names and values).
* `print <variable>` to show a variable `variable`
* `explore <variable>` to inspect a variable `variable`

**Call stack**

* `bt` to show all frames aka the the call stack
*  `select-frame <n>` to select frame number `n` from the call stack

**Program flow**

* `continue` to run the program till the end or the next breakpoint
* `step` to execute the whole line at once
* `next` to execute the next atomic part of the line
* `finish` to return to previous frame

**Getting fancy**

* Consider installing https://github.com/cyrus-and/gdb-dashboard
* Having 2 ssh sessions with one running in gdb `dashboard -output <tty>` can be gold
* `c` is shorthand for `continue`
* `frame` is shorthand for `select-frame`
* `p` is shorthand for `print`

**Debugging a stuck game**

* Open two sessions
* Get one session stuck within a gdb run
* Find the hellband/gdb session with `ps -al`
* Send from the other session a SIGSEGV with `kill -s SIGSEGV <pid>`
