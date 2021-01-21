
docker build -t microbit_ei_build .

docker run -v $PWD:/app -d microbit_ei_build