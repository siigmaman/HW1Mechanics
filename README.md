# Домашнее задание №1 — основы механики

Проект создан на **Unreal Engine 5.8.3** из шаблона Third Person. Для сборки C++ на macOS использован Xcode 26.1.1.

## Что реализовано

- `AHWCharacter` наследуется от `ACharacter`; `BP_ThirdPersonCharacter` использует его как родительский класс.
- Enhanced Input: `IA_Sprint` на Left Shift и `IA_Interact` на E. Привязки событий `Started`, `Completed` и `Canceled` сделаны в C++.
- Спринт меняет `CharacterMovement.MaxWalkSpeed`; флаг `bIsSprinting` доступен в Blueprint.
- `OnInteractionTriggered` — динамический multicast delegate. Его обработчик вызывает событие Blueprint, где узел `Print String` выводит `Interaction triggered!`.
- `ABP_Unarmed` наследуется от `UHWAnimInstance`. В машине `Locomotion` добавлено состояние Sprint с ускоренным воспроизведением анимации бега; переходы зависят от `bIsSprinting`.
- Дополнительно: запас выносливости убывает во время спринта, восстанавливается после остановки и отображается на экране.

## Управление

| Действие | Клавиша |
| --- | --- |
| Движение | WASD |
| Камера | Мышь |
| Прыжок | Space |
| Спринт | Left Shift, удерживать |
| Взаимодействие | E |

## Запуск

1. Установить Unreal Engine 5.8.3. Для сборки C++ на Mac также нужен Xcode 26.1.1 или версия, поддерживаемая UE 5.8.
2. Открыть `HW1Mechanics.uproject` в Unreal Editor и дождаться сборки модулей и шейдеров.
3. Открыть уровень `Content/ThirdPerson/Lvl_ThirdPerson`, если он не открылся автоматически, и нажать **Play**.
4. Удерживать Shift во время движения: скорость и анимация меняются, запас выносливости уменьшается. Отпустить Shift: скорость и анимация возвращаются, запас восстанавливается. Нажать E: появляется `Interaction triggered!`.

## Структура

- `Source/HW1Mechanics` — игровой C++ код персонажа и Animation Instance.
- `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset` — персонаж и вывод сообщения.
- `Content/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.uasset` — анимационная машина со Sprint.
- `Content/Input` — Enhanced Input actions и mapping context.
- `Source/HW1MechanicsEditorTools`, `Editor` — воспроизводимая настройка ресурсов в Unreal Editor.

Папки `Binaries`, `Intermediate`, `Saved` и `DerivedDataCache` создаются движком и не входят в репозиторий.
