# create python virtualenv and install deps

all: install

PIP = .env/bin/pip
PYTHON = .env/bin/python

.env:
	python3 -m venv $@

install: .env requirements.txt
	$(PIP) install -r requirements.txt

receiver:
	$(PYTHON) nats-receiver.py

producer:
	$(PYTHON) nats-producer.py