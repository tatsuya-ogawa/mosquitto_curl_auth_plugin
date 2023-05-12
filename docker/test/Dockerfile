FROM python:latest

WORKDIR /opt/test

COPY ./test.py ./

RUN pip install pytest paho-mqtt

CMD ["python", "-m", "pytest","-s" ,"test.py"]