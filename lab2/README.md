# ПРО — Лабораторна робота №2

**Паралельні та розподілені обчислення.** Синхронізація паралельних операцій за допомогою механізму подій та механізму майбутніх результатів.

| Файл | Пункт | Механізм |
|---|---|---|
| `src/task1_flag.cpp` | 1.2.1 | глобальний прапорець, `unique_lock`, `sleep_for` |
| `src/task2_notify_all.cpp` | 1.2.2 | `condition_variable`, `notify_all` |
| `src/task3_notify_one.cpp` | 1.2.3 | `notify_one` |
| `src/task4_queue_cv.cpp` | 1.2.4 | умовна змінна з предикатом на непорожню чергу |
| `src/task5_async.cpp` | 1.2.5 | `std::async` (`deferred` / `async`) |
| `src/task6_packaged_task.cpp` | 1.2.6 | `packaged_task` у захищених деках, окремий worker |
| `src/task7_promise.cpp` | 1.2.7 | `promise` / `future` без глобальних змінних |

## Збірка та запуск

```bash
make          # зібрати всі програми в bin/
make run      # запустити всі з тестовими даними, вивід -> actual_run.txt
```

Окремий запуск, наприклад: `./bin/task5_async deferred` або `./bin/task5_async async`.
У програмах 1, 4 і 6 введення завершується числом `0`.
