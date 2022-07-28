# YObit-pump-dump

<p align="center">
   <img src="https://img.shields.io/badge/C%2B%2B-14-blue" alt="C++ Version">
   <img src="https://img.shields.io/badge/version-1.0-yellow" alt="Application Version">
   <img src="https://img.shields.io/badge/license-MIT-red" alt="License">
</p>

## This application checks the depth of the Yobit.net exchange.

### About
This console application uses YobitAPIv3.
The application requests tickers from Yobit.net.
It updates the tickers list, if there is a need.
The application shows the trading information and the depth of orders of the chosen ticker.
It remembers the last selected ticker and allows you to quickly receive an information again.

### Documentation
We use the menu to control this console application.
- We view the list of tickers - v
- We update the list of tickers - u
- We get the trading information of the chosen ticker - t
- We repeat the request - r
- Exit - q

We set the number of the ticker and the limit depth (min - 150, max - 2000) when we are requesting the trading information.

### Developers

- [Valendovsky](https://github.com/valendovsky)

### License

Project YObit-pump-dump is distributed under the MIT license.

---

## Приложение позволяет отслеживать наполнение стакана ордеров биржи YObit.net.

### О проекте
Это консольное приложение, которое использует YobitAPIv3.
Программа запрашивает актуальные торговые пары и, если требуется, позволяет обновить их список.
Приложение выводит в консоль основную информацию и состояние стакана ордеров на указанную глубину для выбранной торговой пары.
Запоминает последнюю выбранную пару и позволяет быстро повторно получить информацию по ней.

### Документация
Приложение управляется через меню в консоли:
- показать список актуальных торговых пар - v
- обновить список актуальных торговых пар - u
- запросить информацию по торговой паре - t
- повторно запросить информацию по торговой паре - r
- выход из программы - q

При запросе информации по торговой паре необходимо указать её порядковый номер из списка.
А также указать глубину просмотра стакана ордеров (минимум - 150 ордеров, максимум - 2000 ордеров).

### Разработчики

- [Valendovsky](https://github.com/valendovsky)

### Лицензия
Проект YObit-pump-dump распространяется под лицензией MIT.
