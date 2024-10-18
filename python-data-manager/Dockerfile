FROM python:3

WORKDIR /app

COPY . .

RUN pip install --upgrade pip && pip install -r /app/requirements.txt

CMD ["python", "/app/main.py"]
#CMD "tail -f /dev/null"
