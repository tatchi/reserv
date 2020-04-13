# Reserv

Static file server with auto-reloading written in reason native. This project is heavily inspired by [servor](https://github.com/lukejacksonn/servor) but not as polished and without all the nice features it comes with. I built this mostly for fun and learning so if you need something robust, definitely reach out for something else. There are great alternatives such as [servor](https://github.com/lukejacksonn/servor), [sirv](https://github.com/lukeed/sirv/tree/master/packages/sirv-cli) or [serve](https://github.com/zeit/serve).

## Install

```
$ npm install --save reserv
```

> **Note:** This module can also be installed and used globally

## Usage

```
$ reserv --help
NAME
       reserv - Start a static file server

SYNOPSIS
       reserv [OPTION]... [DIRECTORY]

ARGUMENTS
       DIRECTORY (absent=.)
           Directory to serve

OPTIONS
       -p PORT, --port=PORT (absent=8000)
           Port the server will be running on.

       -r ROOT_FILE, --rootFile=ROOT_FILE (absent=index.html)
           Default file to serve if no path (/).

BUGS
       File bug reports at https://github.com/tatchi/reserv

COMMON OPTIONS
       --help[=FMT] (default=auto)
           Show this help in format FMT. The value FMT must be one of `auto',
           `pager', `groff' or `plain'. With `auto', the format is `pager` or
           `plain' whenever the TERM env var is `dumb' or undefined.

       --version
           Show version information.

```

### Example

```bash
reserv build -p 8000 -r index.html
```