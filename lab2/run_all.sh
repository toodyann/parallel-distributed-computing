#!/usr/bin/env bash
# Запускає всі 7 програм із тестовими даними та зберігає вивід у actual_run.txt
set -e
cd "$(dirname "$0")"
make -s all

{
echo '$ printf "2 4 5 9 11 17 0\n" | ./bin/task1_flag'
printf "2 4 5 9 11 17 0\n" | ./bin/task1_flag
echo
echo '$ ./bin/task2_notify_all'
./bin/task2_notify_all
echo
for i in 1 2 3; do
  echo "\$ ./bin/task3_notify_one  # run $i"
  ./bin/task3_notify_one
done
echo
echo '$ printf "3 4 7 12 13 21 23 0\n" | ./bin/task4_queue_cv'
printf "3 4 7 12 13 21 23 0\n" | ./bin/task4_queue_cv
echo
echo '$ { echo 700000; sleep 1; echo 1; } | ./bin/task5_async deferred'
{ echo 700000; sleep 1; echo 1; } | ./bin/task5_async deferred
echo
echo '$ { echo 700000; sleep 1; echo 1; } | ./bin/task5_async async'
{ echo 700000; sleep 1; echo 1; } | ./bin/task5_async async
echo
echo '$ printf "100 500 1000 0\n" | ./bin/task6_packaged_task'
printf "100 500 1000 0\n" | ./bin/task6_packaged_task
echo
echo '$ printf "250\n" | ./bin/task7_promise'
printf "250\n" | ./bin/task7_promise
} 2>&1 | tee actual_run.txt
