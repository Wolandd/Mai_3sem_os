# Инструкция по компиляции отчета

## Требования

- Установленный LaTeX дистрибутив (TeX Live, MiKTeX или аналогичный)
- Компилятор `pdflatex` или `xelatex`

## Компиляция

Для компиляции отчета выполните в папке `src`:

```bash
cd src
pdflatex main.tex
pdflatex main.tex  # Повторная компиляция для корректного отображения ссылок
```

Или используйте автоматическую компиляцию:

```bash
pdflatex -interaction=nonstopmode main.tex
pdflatex -interaction=nonstopmode main.tex
```

## Структура проекта

```
src/
├── main.tex                    # Главный файл
├── sections/                   # Разделы отчета
│   ├── condition.tex          # Условие (цель, задание, вариант)
│   ├── method.tex             # Метод решения
│   ├── description.tex        # Описание программы
│   ├── results.tex            # Результаты
│   ├── conclusion.tex         # Выводы
│   ├── code.tex               # Исходный код
│   └── strace.tex              # Strace вывод
├── code/                      # Исходный код
│   ├── parent.c
│   └── child.c
└── images/                    # Изображения (если нужны)
```

## Заполнение данных

В файле `main.tex` уже указаны:
- ФИО: Бурмакин М.А.
- Группа: М8О-207БВ-24

При необходимости можно изменить эти данные.

## Результат

После компиляции будет создан файл `main.pdf` с готовым отчетом.

## Получение Strace вывода

Для получения полного вывода strace выполните:

```bash
strace -o strace_output.txt -f ./parent
```

Затем можно вставить содержимое файла `strace_output.txt` в раздел strace отчета.
