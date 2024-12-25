# Task Description
## remote
Напишите сервис, запускающий системные программы по командам, полученных через сетевой сокет. В запросе должно содержаться имя программы и её аргументы (произвольное количество). Протокол взаимодействия - на ваше усмотрение. В ответе должны содержаться код завершения программы, стандартный вывод (в случае успешного завершения), стандартные поток ошибок (в случае ошибки). Необходимо так же предусмотреть таймаут на максимальное время выполнения программы. Все выполняемые программы должны содержаться в системных каталогах (указанных в переменной окружения PATH). Сервис должен иметь возможность одновременного обслуживания множества клиентов. Порт укажите в аргументе командной строки.
Максимальный балл — 20.

# How to Use the Code
1. Склонируйте файлы remote_client.cpp и remote_server.cpp.
2. Вставьте следующую команду в терминал для компиляции со стороны клиента:
```bash
g++ -o remote_client remote_client.cpp -pthread
```
3. Вставьте следующую команду в терминал для компиляции со стороны сервера:
```bash
g++ -o remote_server remote_server.cpp -pthread
```
4. Запустите программу в терминале сначала со стороны сервера:
```bash
./remote_server <port>
```
P.S. Вместо \<port\> введите порт сервера.

5. Запустите программу в терминале со стороны клиента:
```bash
./remote_client <server_ip> <port>
```
P.S. Вместо \<server_ip\> \<port\> введите ip-адрес и порт сервера.

# Requirements
- Компилятор для C++17
- Linux-система
  
# Output
Программа запускает простой аналог для удалённого доступа на сервер. 
Команды, которые вводит клиент, должны выполняться на сервере и перенаправляться клиенту.
После получения результата выполненной команды клиент также получает код завершения от сервера.

# Note
- Поддерживается обслуживание множества клиентов.
