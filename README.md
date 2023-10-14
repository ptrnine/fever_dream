Делаем игрульку-платформер))

# Идея/механики/визуал
За основу берем механику типичного шутера-платформера. ГГ бегает, стреляет всяких чувачков, поднимает предметы на карте.
На эту простую механику пробуем натягивать все, что угодно душе. Чем больше, тем лучше. Вопросов, уместно что-то или нет быть не должно!
Так мы сможем вдоволь поэксперементировать и набраться опыта.
В плане визуала тоже особо не заморачиваемся, рисовать спрайты разрешается хоть карандашом в паинте. Единственное требование тут состоит в том, что спрайт должен быть хотя бы отдаленно похож на то, что он собой представляет.

Мой список идей/референсов выглядит примерно так:

* Трешовый визуал/звуки, isolation, fever dream vibes)) Ориентироваться можно на:
  * [Cruelty Squad](https://store.steampowered.com/app/1388770/Cruelty_Squad/) 
  * [Model of Horror - All lie is salt](https://youtu.be/3DK-c38j9Us)
  * [Half Life - Kleiner Life](https://youtu.be/r6C0FOEdx_8?si=AaOLvnvDb8RX_zSX)
  * Но лучше вообще ни на что не ориентироваться!

* Цепляем физический движок Box2D и делаем рэгдолы, расчлененку ну и прочие приколюхи. Физика в играх добавляет веселья. Стрелять должно быть интересно, а происходящее на экране должно быть захватывающим, ошеломляющим и, возможно, неочевидным. Короче как в перегруженных модах на half-life - пальнул в чувака из какой-то непонятной штуки, а он мало того, что разлетелся в кириешки под перегруженный сэмлп орущего в майнкрафте пацаненка, так из него еще вываливалилась пачка каких-то необъяснимых рэгдолов, застывших в т-позе. Реалистичностью совсем не пахнет, но сложно представить что-то более жестокое.

* Впихиваем диалоги/титры. Не знаю, надо ли оно, но неплохо бы разбавлять визуальную шизу шизой текстовой. Заодно кто-нибудь реализует эффект
телетайпа)) (эффект печатающегося текста)

* Делаем микс из анимаций и физики. В идеале персонажи должны быть максимально восприимчивыми к различным физическим явлениям в игровом мире, пусть даже и смотреться это будет не очень.

# Техничаская часть
Берем SFML для графики/ввода/звуков, также цепляем Box2D для физики. ImGUI или Nuklear берем для UI.

База будет примерно такая:
* Конфиги - сохранение/загрузка настроек, параметров игровых сущностей.
* Уровни/карты - делаем формат карт, возможно, простой редактор, где можно будет размещать области коллизий и спрайты.
* Звуки - сделать обычную воспроизводилку с кэшированием.
* Система сущностей - нужно уметь спаунить сущность по ее названию, задавать ей параметры и т.д. Также сущности должны уметь взаимодействовать друг с другом.
* Система частиц. Сойдет самая простая, которая просто спаунит спрайты различных размеров, скоростей, с физикой или без.
* Возможность создавать скелетные анимации.
* ИИ врагов.

# Как работаем?

Пробуем простейший подход. Для того, чтобы обсудить какую-то идею - делаем задачу. В задаче обсуждаем, что требуется для реализации. Если нам по силам - делаем что нужно, закрываем задачу. Если реализация потребует каких-то непростых изменений в существующем коде - распиливаем на две подзадачи, где каждый будет заниматься тем, что ему по силам.

На данный момент главная цель - написать основной каркас (а-ля симулятор хождения квадрата по плоской карте).

# Work In Progress

stay tuned ёпт
