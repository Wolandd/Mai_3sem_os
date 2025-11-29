# Lab 4 - Проект на C для Linux

## Структура проекта

```
lab/
├── include/
│   └── functions.h          # Заголовочный файл с объявлениями функций
├── src/
│   ├── impl1/               # Первая реализация функций
│   │   ├── sin_integral.c
│   │   ├── translation.c
│   │   └── CMakeLists.txt
│   ├── impl2/               # Вторая реализация функций
│   │   ├── sin_integral.c
│   │   ├── translation.c
│   │   └── CMakeLists.txt
│   ├── program1/            # Программа 1 (использует impl1)
│   │   └── main.c
│   └── program2/            # Программа 2 (использует impl2)
│       └── main.c
├── CMakeLists.txt           # Главный файл сборки
└── README.md
```

## Описание

Проект содержит две реализации функций:
- `SinIntegral` - вычисление интеграла функции sin(x)
- `translation` - перевод числа в двоичное представление

## Сборка

```bash
mkdir build
cd build
cmake ..
make
```

## Запуск

### Program 1 (вычисление интеграла)
```bash
./program1 <A> <B> <e>
```
Пример:
```bash
./program1 0 3.14159 0.001
```

### Program 2 (перевод в двоичную систему)
```bash
./program2 <number>
```
Пример:
```bash
./program2 42
```

