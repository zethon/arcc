# Another Reddit Console Client (`arcc`)

[![Build Status][travis-img]][travis]
[![Build Status][appveyor-img]][appveyor]

## Introduction

While searching for console clients for Reddit, I discovered that of the few tools that existed they were written in Python or Javascript. Variouis tools were required to download and run both, and getting any of them to work was difficult if not impossible. 

`arcc` is console app for reading Reddit. Unlike the others, this one is written in C++, so what does that mean?

* **NO** Javascript modules to install
* **NO** `npm`, `brew`, `someotherthirdpartytool` to deal with
* **NO** BS

Just install and run!

## Third Party Tools

* `boost`<br/>
Boost Software License - http://www.boost.org/users/license.html</br>
http://www.boost.org

* `JSON for Modern C++`<br/>
MIT License - https://github.com/nlohmann/json/blob/develop/LICENSE.MIT<br/>
https://nlohmann.github.io/json/

* `libcurl`<br/>
The curl license - https://curl.haxx.se/docs/copyright.html</br>
https://curl.haxx.se/

* `Lightweight C++ command line option parser`<br/>
MIT License - https://github.com/jarro2783/cxxopts/blob/master/LICENSE<br/>
https://github.com/jarro2783/cxxopts


* `rang`<br/>
The Unlicense - https://github.com/agauniyal/rang/blob/master/LICENSE<br/>
https://github.com/agauniyal/rang

<!-- footnotes -->
[travis-img]: https://travis-ci.org/zethon/arcc.svg?branch=master
[travis]: https://travis-ci.org/zethon/arcc

[appveyor-img]: https://ci.appveyor.com/api/projects/status/goko4jxjkxhmvchq?svg=true
[appveyor]: https://ci.appveyor.com/project/zethon/arcc