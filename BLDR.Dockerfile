from wpierozak/alf-ipbus-env

WORKDIR /

#RUN git clone https://github.com/VictorPierozak/ALFIPbus.git
COPY . /ALFIPbus
WORKDIR ALFIPbus
RUN rm -rf build
RUN mkdir build
RUN git submodule update --init --recursive
RUN cmake -S . -B build/
RUN cmake --build build
RUN cp bin/AlfIPbus /usr/local/bin
