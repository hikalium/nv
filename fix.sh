s=`echo $1 | sed "s/NV_Node_/NV_NodeID_/g"`
sed -i -e "s/$1/$s/g" *.c *.h
