### Введение

Реализован декодер PNG изображений, который соответствует стандартной спецификации и преобразует PNG-изображение в
структуру `Image`, которая хранит матрицу пикселей в формате RGB

Для этого изучил вот [эту](http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html) спецификацию. А именно главы 1-3,
4.1, 5, 6. Где идет поддержка наиболее важных частей PNG

Точка входа — это функция `Image ReadPng(std::string_view filename)`

### Используется

1) Для распаковки данных изображения (дефляции) используется Сишная
   библиотека [libdeflate](https://github.com/ebiggers/libdeflate). Для удобства работы с ней был написан RAII
   класс `DeflateWrapper`
2) Для расчета CRC блока при его валидации используется `boost`
3) Для проверки корректности полученных изображений при тестировании используется библиотека `libpng`
4) Также для тестирования используются  `catch`, подмодули `benchmark` и `googletest`.

### Сборка

Сборка самого декодера находится в `build.cmake`. Сборка тестирования в `CmakeLists.txt`, которая дополнительно включает
декодер.

В папке `tests` находятся PNG картинки для тестирования. В файле `tests.cpp` находится тестирование этих картинок.

### Обработка ошибок

`PNGDecoderException`, `IHDRException`, `DeflateWrapperException`, `BitReaderException` являются
наследниками `std::runtime_error`.

`FailedToReadException` и `InvalidPNGFormatException` являются наследниками `PNGDecoderException`.

Помимо ошибок ниже еще могут вылетать таких же типов, но с другими сообщениями о том, что я что-то куда-то не то передал
и ожидалось получить совсем другое. Это мои внутренние ошибки про то, что я набагал.

| Тип ошибки                  | Ошибка                                                             | Пример сообщения                                                   | Комментарий                                                           |
|-----------------------------|--------------------------------------------------------------------|--------------------------------------------------------------------|-----------------------------------------------------------------------|
| `PNGDecoderException`       | Не получилось открыть файл для чтения png картинки                 | `Unable to open file ...`                                          | `...` имя файла                                                       | 
| `FailedToReadException`     | Ошибка при чтении сигнатуры                                        | `failed to read: "png signature", caught message: ...`             | `...` пойманная ошибка при чтении через std::ifstream                 |
| `FailedToReadException`     | Ошибка при чтении длины данных чанка                               | `*"chunk data length"*`                                            | сообщение и комментарий аналогичен выше                               |
| `FailedToReadException`     | Ошибка при чтении кода типа чанка                                  | `*"chunk type code"*`                                              | ^                                                                     |
| `FailedToReadException`     | Ошибка при чтении данных чанка                                     | `*"chunk data"*`                                                   | ^                                                                     |
| `FailedToReadException`     | Ошибка при чтении CRC чанка                                        | `*"chunk CRC"*`                                                    | ^                                                                     |
| `InvalidPNGFormatException` | Некорректная сигнатура PNG файла                                   | `invalid signature: ...`                                           | `...` считанная сигнатура                                             |
| `InvalidPNGFormatException` | Слишком большая длина данных чанка                                 | `invalid chunk data length: ..., more than 2^31"`                  | `...` прочитанная длина данных чанка                                  |
| `InvalidPNGFormatException` | Некорректный CRC чанка                                             | `invalid CRC: actual = ..., correct = ...`                         | `correct` это то, что мы вычислили, `actual` это то, что мы прочитали |
| `InvalidPNGFormatException` | Не нашли IHDR блок                                                 | `missing chunk "IHDR"`                                             |
| `InvalidPNGFormatException` | Не нашли PLTE блок, хотя палитра используется                      | `missing chunk "PLTE", but palette is used`                        |
| `IHDRException`             | Неправильный размер ihdr данных                                    | `bad read data`                                                    | это моя внутрення ошибка                                              |
| `IHDRException`             | Нулевая длина ширины изображения                                   | `invalid zero width`                                               |
| `IHDRException`             | Нулевая длина высоты изображения                                   | `invalid zero height`                                              |
| `IHDRException`             | Слишком большая ширина изображения                                 | `width = ..., more than 2^31`                                      | `...` прочитанная ширина изображения                                  |
| `IHDRException`             | Слишком большая высота изображения                                 | `height = ..., more than 2^31`                                     | `...` прочитанная высота изображения                                  |
| `IHDRException`             | Некорректная битовая глубина                                       | `invalid bit_depth = ..., != 1, 2, 4, 8 or 16`                     | `...` прочитанная глубина изображения                                 |
| `IHDRException`             | Некорректный тип цвета                                             | `invalid color_type = ..., != 0, 2, 3, 4 or 6 `                    | `...` прочитанный тип цвета                                           |
| `IHDRException`             | Некорректный метод сжатия                                          | `invalid compression_method = ..., != 0`                           | `...` прочитанный метод сжатия                                        |
| `IHDRException`             | Некорректный метод фильтрации                                      | `invalid filter_method = ..., != 0`                                | `...` прочитанный метод фильтрации                                    |
| `IHDRException`             | Некорректный метод чередования пикселей                            | `invalid interlace_method = ..., != 0 or 1`                        | `...` прочитанный метод чередования пикселей                          |
| `DeflateWrapperException`   | Не получилось создать декомпрессор                                 | `bad alloc decompressor`                                           |
| `DeflateWrapperException`   | Не получилось распаковать данные: некорректные данные              | `decompress bad data, see LIBDEFLATE_BAD_DATA`                     |
| `DeflateWrapperException`   | Не получилось распаковать данные: что-то не понятное               | `decompress short output, see LIBDEFLATE_SHORT_OUTPUT`             | никогда не должен вылетать                                            |
| `DeflateWrapperException`   | Не получилось распаковать данные: мало места для записи результата | `decompress insufficient space, see LIBDEFLATE_INSUFFICIENT_SPACE` |                                                                       |
| `InvalidPNGFormatException` | Не хватает пиксельных данных для создания изображения              | `short pixel data length`                                          |
| `InvalidPNGFormatException` | Пиксельных данных больше чем нужно                                 | `too much length pixel data`                                       |
| `InvalidPNGFormatException` | Пиксель-индекс цвета в палитре больше размера палитры              | `pixel index more than palette size`                               |
| `InvalidPNGFormatException` | Некорректный мод фильтра в строке изображения                      | `invalid row filter mode = ..., != 0-4`                            | `...` считанный мод фильтра                                           |