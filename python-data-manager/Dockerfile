FROM python:3

WORKDIR /app

RUN pip install --upgrade pip && pip install -r requirements.txt

COPY . .

CMD tail -f /dev/null