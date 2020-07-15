FROM ubuntu:18.04

RUN apt -y update && apt install -y python3-pip

WORKDIR /app
ADD . .

RUN pip3 install -r requirements.txt

CMD tail -f /dev/null