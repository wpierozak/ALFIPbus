from wpierozak/alf-ipbus-env

WORKDIR /

RUN git clone https://github.com/VictorPierozak/ALFIPbus.git

WORKDIR ALFIPbus
RUN mkdir build
RUN git submodule update --init --recursive
RUN cmake -S . -B build/
RUN cmake --build build
RUN cp bin/AlfIPbus /usr/local/bin
