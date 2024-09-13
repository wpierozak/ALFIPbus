from wpierozak/alf-ipbus-env

WORKDIR /

RUN git clone https://github.com/frun36/alf-ipbus-tester.git

WORKDIR /alf-ipbus-tester
RUN git submodule update --init --recursive
RUN mkdir build
RUN cmake -S . -B build/
RUN cmake --build build
WORKDIR /alf-ipbus/tester/build


ENTRYPOINT [ "/bin/bash" ]