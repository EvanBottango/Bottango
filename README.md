# Bottango - Drivers and API Reference

**Bottango** is an intuitive, visual tool for animating robots, animatronics, and all kinds of hardware. It's free, and can be downloaded at <a href="http://www.Bottango.com/" target="_blank">www.Bottango.com</a>.

The desktop Bottango app runs on a Windows, Linux, or Macintosh computer, and communicates with your hardware using one of the provided open source drivers.

**If you're just getting started with Bottango, I recommend you download the application from the Bottango website, which includes all the drivers and documentation.** The driver code is provided here as well, especially as a route for others to contribute to the development of the driver code.

Documentation for the Bottango application, as well as the open source firmware and examples can be found at the [Bottango Documentation Site](https://docs.bottango.com/).


## Bottango Arduino Driver and Firmware

For most users, this is the most basic and most supported route of communicating with Bottango. This is C++, Arduino framework compatible code to communicate via a serial USB connection from an Arduino framework compatible microcontroller to the desktop app.

Documentation for the Bottango Arduino Driver can be found at the [Bottango Documentation Site: Firmware](https://docs.bottango.com/bottango-firmware/).

## Bottango Networked Driver

Bottango can also communicate with a driver via a web socket connection. Provided here is an example, fully functional implementation of a web socket driver written in Python.

Documentation for the Bottango Networked Driver can be found at the [Bottango Documentation Site: Network Driver](https://docs.bottango.com/apis-and-comms/network-driver/overview/).

## Bottango Driver API Reference

Both the Bottango Arduino Driver and the Bottango Networked Driver are implementations of the Bottango Driver API. You can use this api to understand the communication between the desktop app and the driver, or to build your own driver.

Documentation for the Bottango Driver API can be found at the [Bottango Documentation Site: Bottango Protocol](https://docs.bottango.com/apis-and-comms/bottango-protocol/overview/).

## Bottango REST API

The Bottango Desktop app can be controlled with your own scripts using a rest API, accessed via HTTP. An Example implementation of controlling Bottango via the REST API is provided in Python.

Documentation for the Bottango REST API can be found at the [Bottango Documentation Site: REST API](https://docs.bottango.com/apis-and-comms/rest-api/overview-and-enabling/).

## Copyright and License

BSD 3-Clause License

Copyright (c) 2026, Bottango LLC

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

