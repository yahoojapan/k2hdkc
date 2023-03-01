echo ######################################################
echo ### TEST FOR TEST.SH
echo ######################################################
echo ### SET & PRINT
echo ###
set test-key test-value
print test-key
setsub test-key test-subkey test-sub-value
print test-key
set test-key test-value2 rmsublist
print test-key
print test-subkey
setsub test-key test-subkey test-sub-value
print test-key
set test-key test-value2 rmsub
print test-key
print test-subkey
rm test-key
rm test-subkey

echo ######################################################
echo ### SETSUB & PRINT
echo ###
set test-subkey test-sub-value
set test-key test-value
setsub test-key test-subkey
print test-key
print test-subkey
rm test-key
rm test-subkey

set test-key test-value
setsub test-key test-subkey test-sub-value
print test-key
print test-subkey
rm test-key
rm test-subkey

echo ######################################################
echo ### DIRECTPRINT & DIRECTSET
echo ###
set test-subkey test-sub-value
set test-key test-value
setsub test-key test-subkey
print test-key
print test-key all
directprint test-key 3 5
directset test-key null 5
print test-key
rm test-key
rm test-subkey

echo ######################################################
echo ### REMOVE
echo ###
set test-subkey test-sub-value
set test-key test-value
setsub test-key test-subkey
print test-key all
rm test-key
print test-key
print test-subkey
rm test-subkey

set test-subkey test-sub-value
set test-key test-value
setsub test-key test-subkey
print test-key all
rm test-key all
print test-key
print test-subkey

echo ######################################################
echo ### REMOVE SUB
echo ###
set test-subkey1 test-sub-value
set test-subkey2 test-sub-value
set test-key test-value
setsub test-key test-subkey1
setsub test-key test-subkey2
print test-key all
rmsub test-key all
print test-key
print test-subkey1
print test-subkey2
rm test-key
rm test-subkey1
rm test-subkey2

set test-subkey1 test-sub-value
set test-subkey2 test-sub-value
set test-key test-value
setsub test-key test-subkey1
setsub test-key test-subkey2
print test-key all
rmsub test-key all rmsub
print test-key
print test-subkey1
print test-subkey2
rm test-key
rm test-subkey1
rm test-subkey2

set test-subkey1 test-sub-value
set test-subkey2 test-sub-value
set test-key test-value
setsub test-key test-subkey1
setsub test-key test-subkey2
print test-key all
rmsub test-key test-subkey1
print test-key
print test-subkey1
print test-subkey2
rm test-key
rm test-subkey1
rm test-subkey2

set test-subkey1 test-sub-value
set test-subkey2 test-sub-value
set test-key test-value
setsub test-key test-subkey1
setsub test-key test-subkey2
print test-key all
rmsub test-key test-subkey1 rmsub
print test-key
print test-subkey1
print test-subkey2
rm test-key
rm test-subkey1
rm test-subkey2

echo ######################################################
echo ### FILL
echo ###
fill fill-key fill-value 5
sleep 5
print fill-key-1
print fill-key-2
print fill-key-3
print fill-key-4
print fill-key-5
rm fill-key-1
rm fill-key-2
rm fill-key-3
rm fill-key-4
rm fill-key-5

fillsub fill-parent fill-key fill-value 5
sleep 10
print fill-parent
print fill-key-1
print fill-key-2
print fill-key-3
print fill-key-4
print fill-key-5
rm fill-parent
rm fill-key-1
rm fill-key-2
rm fill-key-3
rm fill-key-4
rm fill-key-5

echo ######################################################
echo ### RENAME
echo ###
set test-key test-value
print test-key
rename test-key test-key-ren
print test-key
print test-key-ren

echo ######################################################
echo ### QUEUE
echo ###
queue queue-key push fifo 000
sleep 1
queue queue-key push fifo 111
sleep 1
queue queue-key push fifo 222
sleep 1
queue queue-key push fifo 333
sleep 1
queue queue-key push fifo 444
sleep 1
queue queue-key pop fifo
sleep 1
queue queue-key pop fifo
sleep 1
queue queue-key remove fifo 2
sleep 1
queue queue-key pop fifo
sleep 1
queue queue-key pop fifo
sleep 1

queue queue-key push lifo 000
sleep 1
queue queue-key push lifo 111
sleep 1
queue queue-key push lifo 222
sleep 1
queue queue-key push lifo 333
sleep 1
queue queue-key push lifo 444
sleep 1
queue queue-key pop lifo
sleep 1
queue queue-key pop lifo
sleep 1
queue queue-key remove lifo 2
sleep 1
queue queue-key pop lifo
sleep 1
queue queue-key pop lifo
sleep 1

echo ######################################################
echo ### KEY QUEUE
echo ###
keyqueue kqueue-key push fifo kq-stack-key0 stack-value0
sleep 1
keyqueue kqueue-key push fifo kq-stack-key1 stack-value1
sleep 1
keyqueue kqueue-key push fifo kq-stack-key2 stack-value2
sleep 1
keyqueue kqueue-key push fifo kq-stack-key3 stack-value3
sleep 1
keyqueue kqueue-key push fifo kq-stack-key4 stack-value4
sleep 1
keyqueue kqueue-key pop fifo
sleep 1
keyqueue kqueue-key pop fifo
sleep 1
keyqueue kqueue-key remove fifo 2
sleep 1
keyqueue kqueue-key pop fifo
sleep 1
keyqueue kqueue-key pop fifo
sleep 1

keyqueue kqueue-key push lifo kq-stack-key0 stack-value0
sleep 1
keyqueue kqueue-key push lifo kq-stack-key1 stack-value1
sleep 1
keyqueue kqueue-key push lifo kq-stack-key2 stack-value2
sleep 1
keyqueue kqueue-key push lifo kq-stack-key3 stack-value3
sleep 1
keyqueue kqueue-key push lifo kq-stack-key4 stack-value4
sleep 1
keyqueue kqueue-key pop lifo
sleep 1
keyqueue kqueue-key pop lifo
sleep 1
keyqueue kqueue-key remove lifo 2
sleep 1
keyqueue kqueue-key pop lifo
sleep 1
keyqueue kqueue-key pop lifo
sleep 1

echo ######################################################
echo ### CAS
echo ###
cas8 init cas8-key 10
cas8 set cas8-key 10 15
cas8 get cas8-key
cas8 set cas8-key 10 20
cas8 get cas8-key
cas8 increment cas8-key
cas8 get cas8-key
cas8 decrement cas8-key
cas8 get cas8-key

cas16 init cas16-key 10
cas16 set cas16-key 10 15
cas16 get cas16-key
cas16 set cas16-key 10 20
cas16 get cas16-key
cas16 increment cas16-key
cas16 get cas16-key
cas16 decrement cas16-key
cas16 get cas16-key

cas32 init cas32-key 10
cas32 set cas32-key 10 15
cas32 get cas32-key
cas32 set cas32-key 10 20
cas32 get cas32-key
cas32 increment cas32-key
cas32 get cas32-key
cas32 decrement cas32-key
cas32 get cas32-key

cas64 init cas64-key 10
cas64 set cas64-key 10 15
cas64 get cas64-key
cas64 set cas64-key 10 20
cas64 get cas64-key
cas64 increment cas64-key
cas64 get cas64-key
cas64 decrement cas64-key
cas64 get cas64-key

echo ######################################################
echo ### END OF TEST
echo ######################################################
exit
