cd "$(dirname "$0")"
cd mjpg-streamer
./mjpg-streamer.sh stop
./mjpg-streamer.sh start

cd ..
cd server
python server.py

