# Конфигурация ADD Server Manager v0.9.91

Документ относится к EXE с SHA-256 `30b210ec80e5733e84ebd05a0cc694eaafedc53289221a0765605113639533c0` и PBO той же версии. [Назад к индексу](../INDEX.md).

## Четыре уровня настроек

| Уровень | Где смотреть / менять | Что настраивает |
| --- | --- | --- |
| Запуск менеджера | `config.yaml` рядом с EXE; раздел Network панели | Адреса, порты, TLS, каталог данных и продолжительность сессий |
| Общие настройки менеджера | Settings; `/api/v1/settings`, секции в SQLite | Steam, обновления, процессы, пути, журналы, уведомления и резервные копии |
| Конкретный сервер DayZ | Manager Config, Configuration, Mods и расписания карточки сервера | Порты, запуск, `serverDZ.cfg`, моды, авторестарт и сбор данных |
| Игровой мод | Bootstrap JSON в профиле DayZ; затем конфиг, получаемый от менеджера | Связь с инстансом, токен, интервалы телеметрии, потоки событий и отчёты |

`settings.cfg` с INI-секциями описан старым README. В проверенной сборке сетевой конфиг и обычные настройки разделены. Не следует превращать таблицу ниже в один INI-файл.

Источники: [Go-структуры и теги полей](../reference/v0.9.91/go-types.json), [SettingsView](../reference/v0.9.91/web-assets/assets/SettingsView-U2cCFYFX.js), [ManagerTab](../reference/v0.9.91/web-assets/assets/ManagerTab-BDKjnhmt.js), [игровые JSON-типы](../reference/v0.9.91/game-mod/Scripts/3_Game/ADD_Server_Manager/Core/ADD_Json.c).

## 1. config.yaml: сеть менеджера

Иллюстративный пример с выбранными значениями, **не извлечённый файл по умолчанию**:

```yaml
listenHost: "127.0.0.1"
listenPort: 8081
ingestPort: 8082
dataDir: "data"
allowedSubnets: []
sessionHours: 12
tls:
  mode: "off"
```

| Поле | Тип | Назначение |
| --- | --- | --- |
| `listenHost` | string | IP-адрес веб-панели; проверка в EXE требует IP, не DNS-имя |
| `listenPort` | int | Порт панели; UI ограничивает диапазоном 1–65535 |
| `ingestPort` | int | Отдельный порт приёма игрового мода; панель сообщает, что он слушает только 127.0.0.1 |
| `dataDir` | string | Каталог данных менеджера |
| `allowedSubnets` | string[] | Список разрешённых подсетей. Точный формат элементов и смысл пустого списка по одному тегу не установлены; не считать этот пример настройкой внешнего доступа |
| `sessionHours` | int | Продолжительность сессии панели в часах; UI требует не менее 1 |
| `tls.mode` | string | `off`, `self-signed`, `file` или `acme` |
| `tls.certFile` / `tls.keyFile` | string | Файлы сертификата и ключа для режима `file` |
| `tls.domain` / `tls.email` | string | Домен и почта для режима `acme` / Let's Encrypt |

Сеть в панели читается через `GET /api/v1/system/network` и записывается через `PUT /api/v1/system/network`. Панель явно требует перезапустить менеджер для применения адреса и TLS. Перезапуск игрового инстанса — другая операция.

## 2. Общие настройки менеджера

В EXE есть SQL `SELECT section, data FROM settings` и `INSERT INTO settings(section,data) ... ON CONFLICT`. Обычные настройки хранятся по секциям в БД. `settings.Store` содержит ссылку на БД и текущую структуру `settings.All`.

Панель использует:

- `GET /api/v1/settings` — получить все секции;
- `PUT /api/v1/settings/<section>` — сохранить объект конкретной секции;
- `POST /api/v1/settings/<section>/reset` — вернуть секцию к значениям по умолчанию;
- `PUT /api/v1/settings-secrets` — отдельно сохранить секреты.

Ниже перечислены **все поля 16 секций `settings.All`**. Типы, перечисления и границы взяты из метаданных; отсутствие границы не означает, что backend допускает любое значение. Значения по умолчанию для этих секций статически не восстановлены. Числовые `minimum` и `maximum` — аннотации API, не проверенные нами на запущенном сервере.

Пример тела для `PUT /api/v1/settings/general`, не самостоятельный конфигурационный файл:

```json
{
  "language": "ru",
  "logLevel": "info",
  "timezone": "Europe/Moscow"
}
```

### `general` — Общие настройки

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `language` | string; варианты: ru,en | Язык интерфейса/сообщений; `ru` или `en` в зависимости от секции. |
| `logLevel` | string; варианты: debug,info,warn,error | Подробность логов менеджера: `debug`, `info`, `warn`, `error`. |
| `timezone` | string | Часовой пояс расписаний: `Local`, `UTC`, смещение вроде `+03:00` или IANA-имя, например `Europe/Moscow`. |

### `paths` — Пути

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `serversPath` | string | Папка серверов |
| `steamCmdPath` | string | SteamCMD |
| `logsPath` | string | Логи |
| `versionsPath` | string | Версии |
| `backupFolder` | string | Имя папки резервных копий файлов внутри каждого корня игрового сервера; не интервал резервного копирования БД. |

### `steam` — Учётная запись Steam

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `username` | string | Логин Steam |

### `steamcmd` — Работа SteamCMD

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `downloadTimeout` | int; min: 30 | Таймаут скачивания SteamCMD. Единицу измерения Go-тег не содержит; перед изменением сверить с API/работающим приложением. |
| `retryCount` | int; min: 0; max: 10 | Повторы |
| `modsMaxBatchSize` | int; min: 1; max: 50 | Мод в пакете |

### `updates` — Обновления

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `checkModUpdatesInterval` | int; min: 0 | Проверка модов, сек |
| `autoUpdateMods` | bool | Скачивать моды автоматически |
| `autoUpdateModsOnStartup` | bool | Проверять моды при запуске |
| `checkServerUpdatesInterval` | int; min: 0 | Проверка DayZ Server, сек |
| `autoUpdateServers` | bool | Обновлять DayZ Server автоматически |
| `autoUpdateServerOnStartup` | bool | Проверять DayZ Server при запуске |
| `autoUpdateInstancesOnStartup` | bool | Обновлять файлы серверов при запуске |
| `modUpdateRestartDelay` | int; min: 0 | Задержка рестарта после обновления мода, секунды. |
| `modUpdateMessageRestartSec` | int; min: 5 | Интервал отправки сообщений об обновлении, сек |
| `modUpdateKickRestartSec` | int; min: 0 | Кик перед обновлением, сек |
| `modUpdateLockRestartSec` | int; min: 0 | Блокировка сервера перед обновлением, сек |
| `mergeWithScheduledWindowSec` | int; min: 0 | Окно объединения рестарта из-за обновления с плановым рестартом, секунды. |
| `checkModsBeforeScheduledRestartSec` | int; min: 0 | Проверка модов перед рестартом, сек |
| `deepVerifyIntervalSec` | int; min: 0 | Интервал глубокой проверки модов, секунды. Поведение нуля требует проверки работающего приложения. |
| `keepGameDeltas` | int; min: 1 | Сколько дельт версий игры хранить; дельта — различия между версиями, а не отдельный игровой инстанс. |

### `server` — Запуск и восстановление DayZ

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `executableName` | string | Исполняемый файл |
| `configName` | string | Файл конфига |
| `startupTimeout` | int; min: 5 | Ожидание запуска, сек |
| `shutdownTimeout` | int; min: 1 | Ожидание остановки, сек |
| `shutdownAttempts` | int; min: 1 | Попытки остановки |
| `useTaskkillOnStop` | bool | Завершать процесс при остановке |
| `enableHangDetection` | bool | Проверка зависания |
| `hangCheckIntervalSec` | int; min: 5 | Интервал проверки зависания, сек |
| `hangMaxNoResponseChecks` | int; min: 1 | Проверок без ответа |
| `autoRestartCrashed` | bool | Перезапускать после падения |
| `autoRestartDelay` | int; min: 0 | Пауза перед перезапуском, сек |
| `enableCrashLoopGuard` | bool | Защита от частых падений |
| `crashLoopThreshold` | int; min: 2 | Падений для паузы |
| `crashLoopWindowSec` | int; min: 30 | Окно падений, сек |
| `autostartDelay` | int; min: 0 | Пауза автозапуска, сек |

### `process` — Процесс менеджера

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `gracePeriod` | int; min: 0 | Пауза после старта, сек |
| `lowCpuThreshold` | float64; min: 0 | Порог CPU зависания, % |
| `lowCpuTimeoutOverride` | int; min: 0 | Таймаут при низкой загрузке CPU, сек |
| `elevateLevel` | string; варианты: low,below_normal,normal,above_normal,high,realtime | Приоритет процесса самого менеджера: `low`, `below_normal`, `normal`, `above_normal`, `high`, `realtime`; это не выбор учётной записи и не настройки сервера DayZ. |
| `affinityMask` | string | Десятичная строка с битовой маской ядер самого менеджера; пустая строка означает все ядра. Например, `"3"` выбирает ядра 0 и 1. |

### `logrotate` — Логи и срок хранения служебных данных

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `appRotateMinutes` | int; min: 0 | Ротация логов менеджера, мин |
| `appKeepFiles` | int; min: 1 | Хранить файлов логов менеджера |
| `auditKeepDays` | int; min: 1; max: 90 | Хранить аудит, дней |
| `jobsKeepDays` | int; min: 1; max: 90 | Хранить завершённые задачи, дней |
| `hostKeepDays` | int; min: 1; max: 30 | Хранить нагрузку компьютера и доставки, дней |

### `messages` — Сообщения в игре

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `language` | string; варианты: ru,en | Язык интерфейса/сообщений; `ru` или `en` в зависимости от секции. |
| `restartWarning` | map[string]string | Предупреждение о рестарте. Словарь языков, например `{"ru":"...","en":"..."}`. |
| `restartNow` | map[string]string | Рестарт сейчас. Словарь языков, например `{"ru":"...","en":"..."}`. |
| `modUpdateWarning` | map[string]string | Обновление мода. Словарь языков, например `{"ru":"...","en":"..."}`. |
| `modUpdateKickMessage` | map[string]string | Причина кика при обновлении. Словарь языков, например `{"ru":"...","en":"..."}`. |
| `kickBeforeRestart` | map[string]string | Сообщение перед киком. Словарь языков, например `{"ru":"...","en":"..."}`. |
| `kickReasonBeforeRestart` | map[string]string | Причина кика. Словарь языков, например `{"ru":"...","en":"..."}`. |
| `playerKickMessage` | map[string]string | Кик игрока. Словарь языков, например `{"ru":"...","en":"..."}`. |
| `lockBeforeRestart` | map[string]string | Сообщение при закрытии перед рестартом. Словарь языков, например `{"ru":"...","en":"..."}`. |
| `serverUpdateRestartMessage` | map[string]string | Обновление игры. Словарь языков, например `{"ru":"...","en":"..."}`. |

### `tools` — Редактирование файлов

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `fileEditBackup` | bool | Копия файла перед правкой |
| `maxFileSizeKB` | int; min: 16 | Максимальный размер файла, КБ |

### `ingest` — Приём игрового мода

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `enabled` | bool | Ingest включён |
| `killThresholdNotify` | bool | Включение уведомлений о серии убийств; здесь это boolean, числового порога в этой секции нет. |

### `journal` — Журнал игровых событий

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `retentionDays` | int; min: 1; max: 3650 | Срок хранения журнала в днях. По метаданным очищается не весь остаток сразу: удаляется по одному старому календарному дню в сутки. |
| `queueBatches` | int; min: 16; max: 10000 | Число пачек журнала, хранимых в памяти до выгрузки на диск; увеличение повышает потребление RAM. |

### `notifications` — Уведомления

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `discordWebhookSet` | bool; только чтение: true | Признак наличия секрета Discord; поле только для чтения, не URL webhook. |
| `telegramTokenSet` | bool; только чтение: true | Признак наличия токена Telegram; поле только для чтения, не сам токен. |
| `telegramChatId` | string | ID чата Telegram |
| `language` | string; варианты: ru,en | Язык интерфейса/сообщений; `ru` или `en` в зависимости от секции. |

### `firewall` — Брандмауэр Windows

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `autoCreateRules` | bool | Создавать правила автоматически |

### `players` — Сведения об игроках

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `geoIpEnabled` | bool | Определять страну по IP |
| `geoIpAutoUpdate` | bool | Ежемесячно загружать DB-IP Lite; точное поведение подтверждено описанием поля в метаданных. |
| `geoIpFile` | string | Путь к собственной `.mmdb` MaxMind или DB-IP; пустой путь означает скачанный файл. |
| `maskIps` | bool | Скрывать IP от пользователей без разрешения `players.pii`. Подпись в русской панели сформулирована иначе; здесь использовано пояснение из Go-тега. |

### `backups` — Резервные копии баз данных

| Поле | Тип / ограничения | Значение |
| --- | --- | --- |
| `intervalHours` | int; min: 1; max: 720 | Копия manager.db, раз в часов |
| `keep` | int; min: 1; max: 100 | Хранить копий manager.db |
| `journalIntervalHours` | int; min: 1; max: 720 | Копия журнала, раз в часов |
| `journalKeep` | int; min: 1; max: 100 | Хранить копий журнала |
| `playerIntervalHours` | int; min: 1; max: 720 | Копия player.db, раз в часов |
| `playerKeep` | int; min: 1; max: 100 | Хранить копий player.db |

### Секреты

Пароль Steam и Steam Web API key не входят в `settings.Steam`, где есть только `username`. Панель записывает `steamPassword`, `steamApiKey`, `discordWebhook`, `telegramToken` отдельно через `settings-secrets`. Пустое поле не заменяет сохранённый секрет. В Go-метаданных есть операция с `clear` — массивом имён удаляемых секретов; точную форму использовать по API работающего приложения.

Модуль `platform/secrets/dpapi_windows.go` и текст SetupView подтверждают DPAPI. Служба Windows запускается под той же учётной записью для расшифровки данных. `sessionHours` здесь не пароль RCon; пароль RCon относится к инстансу и редактируется отдельно.

### Единицы и неоднозначные настройки

Имена с `Sec` и `Ms` явно обозначают секунды и миллисекунды. Для остальных полей единицы указаны по тексту панели; где их нет, они не угаданы. Например, для `steamcmd.downloadTimeout` установлен нижний предел 30, но единица в исследованных тегах и подписи поля отсутствует.

`process.affinityMask` управляет менеджером; `instance.affinityMask` — процессом сервера. `backups.*` задаёт копии БД; `paths.backupFolder` и `tools.fileEditBackup` относятся к копиям редактируемых файлов. `autoUpdateMods` управляет скачиванием, а расписание перезапусков и предупреждения задаются отдельно.

## 3. Настройки конкретного инстанса

Встроенный `ManagerTab` читает следующие поля. Это описание формы v0.9.91, не полная схема БД и не готовый импортный JSON.

| Поле / группа | Значение |
| --- | --- |
| `id`, `createdAt`, `updatedAt` | Идентификатор и служебные даты |
| `name`, `maxPlayers`, `missionTemplate`, `queryPort` | В BasicSection показаны как управляемые из `serverDZ.cfg`; исправлять через Configuration |
| `order`, `enabled`, `autostart` | Порядок, активность и запуск вместе с менеджером |
| `port` | Игровой порт; форма проверяет конфликты портов |
| `rconPort`, `restrictRCon`, пароль RCon | Канал BattlEye; пароль читается отдельным запросом, не обычным JSON-полем инстанса |
| `executablePath` | Путь к исполняемому файлу DayZ |
| `profilesDir`, `battlEyePath` | Каталоги профиля DayZ и BattlEye |
| `priority`, `affinityMask` | Приоритет и ядра процесса этого игрового сервера |
| `map` | Выбор карты для отображения; в форме есть отдельный каталог `/maps` |
| `launchParams` | Массив дополнительных параметров запуска |
| Моды | Workshop ID, порядок, enabled и server/client; отдельная вкладка Mods |
| Расписания | Cron-рестарты и сообщения, настройки обновлений/уведомлений и мода — отдельные секции карточки |

Параметры `-config`, `-port`, `-profiles`, `-bepath`, `-mod`, `-servermod` форма исключает из пользовательского `launchParams`: их формирует менеджер. Не нужно дублировать их вручную. Предпросмотр итоговой команды получается через `/api/v1/instances/<id>/launch-preview`.

Cron-редактор использует пять полей: `минута час день-месяца месяц день-недели`. Пример `0 */6 * * *` — каждые шесть часов в нулевую минуту в выбранной таймзоне. В разрешённых UI-контекстах также есть `@startup`; переносить его во все виды расписаний без проверки не следует.

## 4. Bootstrap игрового мода

Точный путь из кода: `$profile:ADDEV/ADD_Server_Manager/ADD_Server_ManagerSetting.json`. `$profile:` — каталог, выбранный параметром запуска DayZ `-profiles`, не каталог профиля Windows.

```json
{
  "managerUrl": "http://127.0.0.1:8082",
  "instanceId": "IDENTIFIER_FROM_MANAGER",
  "token": "TOKEN_FROM_MANAGER"
}
```

| Поле | Значение |
| --- | --- |
| `managerUrl` | Адрес ingest, а не URL веб-панели; значение 8082 — реальный fallback в коде мода |
| `instanceId` | ID инстанса, по которому менеджер связывает события с игровым сервером |
| `token` | Токен связи мода с менеджером; не пароль Steam, панели или RCon |

При отсутствии файла мод создаёт его с пустыми ID и токеном. Само создание файла не завершает привязку инстанса. HTTP-транспорт использует `/ingest/v2/<instanceId>/...`, Bearer-заголовок и параметр `?token=`; подробнее в [ADD_Http.c](../reference/v0.9.91/game-mod/Scripts/3_Game/ADD_Server_Manager/Transport/ADD_Http.c).

После bootstrap мод запрашивает конфигурацию у менеджера и читает объект `{ "config": ... }`. Следующая таблица показывает **дефолты конструктора мода**, а не гарантированные значения, которые менеджер вернёт этому серверу:

| Поле | Дефолт | Значение |
| --- | --- | --- |
| `version` | 1 | Версия конфигурации; значения ≤0 при получении отклоняются |
| `presenceDetail` | true | Детализация данных присутствия |
| `intervalsSec.heartbeat` | 30 | Период heartbeat, секунды |
| `intervalsSec.online` | 30 | Период обновления онлайн-списка, секунды |
| `intervalsSec.positions` | 5 | Период отправки позиций, секунды |
| `positions.minMoveMeters` | 2 | Минимальное перемещение для обновления позиции, метры |
| `positions.staleSec` | 60 | Порог устаревания позиции, секунды |
| `snapshots.intervalSec` | 60 | Период полных снимков отслеживаемых объектов, секунды |
| `snapshots.changeScanSec` | 5 | Период поиска изменений, секунды |
| `journal.flushMs` | 1500 | Период сброса пачки событий, миллисекунды |
| `journal.moveMeters` | 0.5 | Порог движения для событий журнала, метры |
| `journal.vehicleMinDamage` | 2 | Минимальный урон транспорту для события |
| `journal.streams` | Все перечисленные флаги true | Какие потоки собирать и отправлять |
| `journal.actions` | Шесть классов действий ниже | Фильтр журналируемых действий; пустой массив означает все |
| `journal.recipes` | [] | Пустой массив означает все рецепты |
| `combat.hitMinDamage` | 0.5 | Порог урона для регистрации попадания |
| `combat.magicBullet` | true | Флаг обработки, названной автором magicBullet; это не доказанная нами античит-защита |
| `reports.enabled` | true | Игровые отчёты включены |
| `reports.cooldownSec` | 30 | Пауза между отчётами, секунды |
| `reports.maxMessageLen` | 500 | Максимальная длина сообщения отчёта |
| `reports.targetRadiusM` | 50 | Радиус поиска цели отчёта, метры |
| `trackedItemClasses` | TentBase, Barrel_ColorBase, UndergroundStash | Отслеживаемые классы предметов/хранилищ |

Дефолтный `journal.actions`: `ActionBuryBody`, `ActionStartEngine`, `ActionRepairCarEngineWithBlowtorch`, `ActionPlaceObject`, `ActionDeployObject`, `ActionPlaceOnGround`.

`Validate()` корректирует часть значений: heartbeat <5 становится 5; online <5 становится 30; positions-интервал <1 становится 1; snapshot-интервал <10 становится 60; changeScan <1 становится 5; flush <500 становится 1500. Отрицательные пороги и слишком короткие поля отчётов также исправляются по коду. Это не универсальное ограничение всех полей: например, не каждому интервалу задана верхняя граница.

### Переключатели journal.streams

Все 37 флагов из `ADD_Streams` по умолчанию true:

| Группа | Поля |
| --- | --- |
| Бой и здоровье | `playerHit`, `disease`, `brokenLegs`, `unconscious`, `cuts`, `death`, `deathItems` |
| Сессии и чат | `session`, `chat` |
| Предметы и действия | `itemMove`, `itemDamage`, `action`, `recipe` |
| Автомобили | `carDamage`, `carUse`, `carSpawn`, `carDelete`, `carMove` |
| Лодки | `boatDamage`, `boatUse`, `boatSpawn`, `boatDelete`, `boatMove` |
| Стройки и базы | `build`, `dismantle`, `flagBuild`, `baseGone`, `baseChange`, `trackedItems` |
| Мир | `zombie`, `animal`, `gas`, `crash`, `corpse` |
| Телеметрия | `heartbeat`, `online`, `positions` |

Счётчик проверяется по исходному списку полей, не по названию группы. Выключенный поток не собирается и не отправляется. `crash` в группе мира не следует автоматически понимать как падение процесса DayZ: это имя игрового потока, а обнаружение падения процесса относится к `settings.Server`.

## Что пока не установлено

- Фактические значения по умолчанию всех глобальных секций и поведение каждого нулевого интервала.
- Полная схема persisted JSON инстанса и все обработчики API backend.
- Точные команды сборки менеджера и frontend: их исходные проекты отсутствуют.
- Декодированный `config.cpp` PBO, требования к упаковке, подписи и полный состав клиентской установки.
- Результаты запуска, установки службы, работы Steam и нагрузочные характеристики на Windows.

Файлы в `reference` — зафиксированные материалы релиза. Редактирование JS/CSS в этом каталоге само по себе не меняет EXE; редактирование `.c` не меняет опубликованный PBO до его отдельной сборки.
