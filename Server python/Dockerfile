FROM python:3

WORKDIR /app

COPY . /app
VOLUME [ "./:/app"]

RUN pip install --no-cache-dir -r requirements.txt


ENTRYPOINT ["python3","app.py"]