# Домашние задания №1–2 — механики, Behavior Tree и PCG

Проект создан на **Unreal Engine 5.8.3** из шаблона Third Person. Для сборки C++ на macOS использован Xcode 26.1.1.

## Что реализовано

### ДЗ1: механики персонажа

- `AHWCharacter` наследуется от `ACharacter`; `BP_ThirdPersonCharacter` использует его как родительский класс.
- Enhanced Input: `IA_Sprint` на Left Shift и `IA_Interact` на E. Привязки событий `Started`, `Completed` и `Canceled` сделаны в C++.
- Спринт меняет `CharacterMovement.MaxWalkSpeed`; флаг `bIsSprinting` доступен в Blueprint.
- `OnInteractionTriggered` — динамический multicast delegate. Его обработчик вызывает событие Blueprint, где узел `Print String` выводит `Interaction triggered!`.
- `ABP_Unarmed` наследуется от `UHWAnimInstance`. В машине `Locomotion` добавлено состояние Sprint с ускоренным воспроизведением анимации бега; переходы зависят от `bIsSprinting`.
- Дополнительно: запас выносливости убывает во время спринта, восстанавливается после остановки и отображается на экране.

### ДЗ2: окружение и искусственный интеллект

- В уровень добавлен `NavMeshBoundsVolume`, закрывающий всю игровую арену. Навигация работает в динамическом режиме и учитывает процедурные препятствия.
- `PCG_ArenaProps` создаёт сетку точек, добавляет случайные смещения, повороты и масштаб, после чего размещает несколько видов пропсов через `Static Mesh Spawner`. Центр арены оставлен свободным для PlayerStart.
- `AHWAICharacter` — отдельный C++-персонаж ИИ с моделью Quinn, а `AHWAIController` запускает Blackboard и Behavior Tree и использует `AIPerception Sight`.
- `BB_Enemy` хранит цель, стартовую точку, точку патруля, точку побега и флаг состояния побега.
- `BT_Enemy` переключает три ветки по приоритету: **Flee → Chase → Patrol**.
- Патрулирование выбирает случайную достижимую точку вокруг стартовой позиции. При обнаружении игрока ИИ прерывает патруль и преследует цель; после потери видимости возвращается к патрулю.

#### Уникальная механика на 9–10 баллов

При нажатии игроком **E** ближайший ИИ на четыре секунды переходит в состояние **Flee**. Кастомный C++-узел Behavior Tree `UHWBTTask_SetFleeLocation` вычисляет направление от угрозы, добавляет небольшое случайное отклонение, проецирует точку на NavMesh и передаёт её ветке движения. Реакция использует делегат взаимодействия из ДЗ1, поэтому обе домашние работы связаны общей механикой.

## Управление

| Действие | Клавиша |
| --- | --- |
| Движение | WASD |
| Камера | Мышь |
| Прыжок | Space |
| Спринт | Left Shift, удерживать |
| Взаимодействие / заставить ближайший ИИ убегать | E |

## Запуск

1. Установить Unreal Engine 5.8.3. Для сборки C++ на Mac также нужен Xcode 26.1.1 или версия, поддерживаемая UE 5.8.
2. Открыть `HW1Mechanics.uproject` в Unreal Editor и дождаться сборки модулей и шейдеров.
3. Открыть уровень `Content/ThirdPerson/Lvl_ThirdPerson`, если он не открылся автоматически, и нажать **Play**.
4. Нажать **Play**. ИИ патрулирует арену, преследует игрока в поле зрения и возвращается к патрулю после потери цели.
5. Подойти к ИИ на расстояние до 14 метров и нажать **E**: появляется `Interaction triggered!`, а ИИ прерывает текущую ветку и убегает.
6. Удерживать Shift во время движения: скорость и анимация меняются, запас выносливости уменьшается. Отпустить Shift: скорость и анимация возвращаются, запас восстанавливается.

## Структура

- `Source/HW1Mechanics` — игровой C++ код персонажа и Animation Instance.
- `Source/HW1Mechanics/HWAI*` — персонаж и контроллер искусственного интеллекта.
- `Source/HW1Mechanics/HWBT*` — кастомные Task и Decorator для Behavior Tree.
- `Content/HW2/AI` — Blackboard и Behavior Tree.
- `Content/HW2/PCG` — граф процедурной генерации окружения.
- `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset` — персонаж и вывод сообщения.
- `Content/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.uasset` — анимационная машина со Sprint.
- `Content/Input` — Enhanced Input actions и mapping context.
- `Source/HW1MechanicsEditorTools`, `Editor` — воспроизводимая настройка ресурсов в Unreal Editor, включая `setup_hw2.py`.

Папки `Binaries`, `Intermediate`, `Saved` и `DerivedDataCache` создаются движком и не входят в репозиторий.
