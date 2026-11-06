# hw-1 · Разминка CI — Сетевые технологии

[![CI](https://github.com/USERNAME/REPO/actions/workflows/ci.yml/badge.svg)](https://github.com/USERNAME/REPO/actions/workflows/ci.yml)

> После создания репозитория замените `USERNAME/REPO` в строке бейджа
> выше на свои логин и имя репозитория — она станет зелёным бейджем сборки.

Реализованы три функции по интерфейсу `include/hw1.h`:

| Функция         | Назначение                     | Контракт                                                                                                                            |
| --------------- | ------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------- |
| `parse_mac`     | разбор строки MAC в 6 байт     | разделители `:` и `-` (можно вперемешку), любой регистр; `0` — успех, `-1` — любая ошибка (NULL, неверная длина, не-hex символ, лишние символы) |
| `format_mac`    | вывод MAC в каноническом виде  | `AA:BB:CC:DD:EE:FF`, только заглавные буквы, `'\0'` в конце; `out` не меньше 18 байт                                                  |
| `ipv4_checksum` | контрольная сумма заголовка    | RFC 1071: дополнение до единицы суммы 16-битных слов (big-endian); поле checksum в буфере заранее обнулено; нечётная длина — последний байт дополняется нулевым младшим байтом |

## Структура репозитория

```text
.
├── .github/workflows/ci.yml   # CI: матрица {gcc, clang} x {Debug, Release} + ASan/UBSan + valgrind
├── include/hw1.h              # интерфейс задания — не изменять
├── src/hw1.c                  # реализация трёх функций
├── tests/test_hw1.c           # тесты: публичные сценарии + краевые случаи
└── CMakeLists.txt             # сборка, ctest и опция ENABLE_SANITIZERS
```

## Локальная сборка и тесты

```bash
cmake -B build                                 # конфигурация (по умолчанию Debug)
cmake --build build                            # сборка
ctest --test-dir build --output-on-failure     # тесты
```

Другой компилятор или конфигурация:

```bash
CC=clang cmake -B build-clang -DCMAKE_BUILD_TYPE=Release
cmake --build build-clang
ctest --test-dir build-clang --output-on-failure
```

Санитайзеры (ASan + UBSan + LeakSan) — опция из `CMakeLists.txt`, включается
одним флагом конфигурации:

```bash
cmake -B build-asan -DENABLE_SANITIZERS=ON
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

Valgrind — независимая от компилятора проверка памяти и утечек (в CI
ставится автоматически, локально нужен установленный valgrind):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
valgrind --leak-check=full --show-leak-kinds=all \
         --errors-for-leak-kinds=definite,indirect \
         --track-origins=yes --error-exitcode=1 \
         ./build/test_hw1
```

Обвязка CTest прогонит под valgrind сразу все зарегистрированные тесты:
сконфигурируйте с `-DCMAKE_MEMORYCHECK_COMMAND=/usr/bin/valgrind` и
запустите `ctest --test-dir build -T memcheck --output-on-failure`.

## CI на GitHub Actions

`.github/workflows/ci.yml` запускается на каждый `push` (в любую ветку)
и на каждый `pull_request`. Всего 6 джобов:

- **build-and-test** — матрица gcc / clang × Debug / Release (4 джоба):
  сборка с `-Wall -Wextra -Werror` (флаги заданы в `CMakeLists.txt`,
  поэтому действуют и локально, и в CI) и прогон `ctest`;
- **sanitizers** — сборка с `-DENABLE_SANITIZERS=ON`
  (`-fsanitize=address,undefined`): ASan ловит выходы за границы памяти
  и use-after-free, UBSan — неопределённое поведение, LeakSanitizer —
  утечки (флаги `ASAN_OPTIONS`/`UBSAN_OPTIONS` в джобе);
- **valgrind** — прогон тестов под `valgrind --leak-check=full
  --error-exitcode=1`: джоб краснеет при любой ошибке памяти или утечке.

Компиляторы предустановлены на образе `ubuntu-latest`; valgrind workflow
ставит сам через `apt-get install`.

## Как сдать задание (git и GitHub)

1. На github.com: **New repository** → имя (например, `networking-hw1`) →
   **без** README, .gitignore и license (файлы уже в этом каркасе) → Create.
2. Из корня распакованного каталога:

   ```bash
   git init
   git add .
   git commit -m "hw1: parse_mac, format_mac, ipv4_checksum + CI"
   git branch -M main
   git remote add origin git@github.com:USERNAME/REPO.git   # или https://github.com/USERNAME/REPO.git
   git push -u origin main
   ```

3. Откройте вкладку **Actions** — после push там должны позеленеть
   все 6 джобов.
4. Замените `USERNAME/REPO` в бейдже в начале этого файла.

## Что проверяет преподаватель

1. **Сборка по стандартному пути** — `cmake -B build && cmake --build build && ctest`
   проходит на чистом клоне без каких-либо правок.
2. **Скрытые тесты против `src/hw1.c`** — NULL-указатели, обрезанные
   строки, строки с лишними символами, нечётный буфер. Все эти случаи
   уже покрыты локальными тестами из `tests/test_hw1.c`.
3. **Зелёный CI** — вкладка Actions в вашем репозитории.

Жизненный цикл сдачи из задания: clone → реализация → локальный ctest →
push → зелёный CI.
