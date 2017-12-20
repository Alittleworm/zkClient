#请先解压zookeeper-3.4.10.tar.gz: tar -xzvf ./zookeeper-3.4.10.tar.gz
ln -s ./zookeeper-3.4.10 ./zookeeper
cd ./zookeeper/src/c
./configure
make
make install
cd .libs
cp -f ./libhashtable.a /data/gameserver/libs/
cp -f ./libzookeeper_mt.so.2.0.0 /data/gameserver/libs/
cp -f ./libzookeeper_st.so.2.0.0 /data/gameserver/libs/
cd /data/gameserver/libs
ln -s -f ./libzookeeper_mt.so.2.0.0 ././libzookeeper_mt.so	
ln -s -f ./libzookeeper_st.so.2.0.0 ././libzookeeper_st.so	
