make clean;make;
sudo cp -f ./libCppZooKeeper.so /data/gameserver/libs/libCppZooKeeper.so.1.0.4
sudo cp -f ./libCppZooKeeper.a /data/gameserver/libs/libCppZooKeeper.a
cd /data/gameserver/libs
sudo ln -s -f ./libCppZooKeeper.so.1.0.4 ./libCppZooKeeper.so
