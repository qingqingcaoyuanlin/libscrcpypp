FROM ubuntu:noble
LABEL authors="ender"
RUN apt-get update
RUN apt-get install libboost-all-dev cmake g++-14 gcc-14 gdb git nano pkg-config ninja-build make -y
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 30 --slave /usr/bin/g++ g++ /usr/bin/g++-14
RUN git clone https://github.com/EnderTheCoder/libscrcpypp
RUN mkdir libscrcpypp/build
WORKDIR /libscrcpypp/build
RUN cmake ..
RUN cmake --build . -j
RUN cmake --install .