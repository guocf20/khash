#/*=================================
#* Copyright(C)2018 All rights reserved
#author:guochengfeng
#*last modify:2018--06--08
#*email:guocf20@gmail.com
#*=================================*/   
if [ $1 = "build" ]; then
gcc set.c -o set
gcc get.c -o get
exit 0
fi
rm -rf set get


