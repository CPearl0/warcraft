#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <any>
#include <iomanip>
#include <math.h>

namespace warcraft {
    enum class camp_label: int8_t {
        red = 0,
        blue = 1
    };

    enum class game_message: int8_t {
        stop_generate_warrior
    };

    const std::string camp_name(camp_label camp) noexcept
    {
        switch (camp) {
            case camp_label::red:
                return "red";
            case camp_label::blue:
                return "blue";
        }
    }

    int hour(int time) noexcept
    {
        return time / 60;
    }

    int minute(int time) noexcept
    {
        return time % 60;
    }

    class game_controller;

    class game_object {
    public:
        virtual ~game_object() = default;
        virtual void on_update_time(int new_time) {}
    };

    class warrior;

    class city: public game_object {
    public:
        virtual ~city() = default;
    };

    constexpr int warrior_type_count = 5;

    class headquarter: public city {
    protected:
        game_controller* _controller;

        camp_label _camp;
        int _health_point;
        std::vector<std::unique_ptr<warrior>> _warriors; // RAII

        int _last_generate_warrior = -1;
        std::array<int, warrior_type_count> _warrior_record{ 0 };
        bool _stopped = false;
    public:
        headquarter(camp_label camp, int health_point, game_controller* controller) noexcept
            : _camp(camp), _health_point(health_point), _controller(controller)
        {
        }
        virtual ~headquarter() = default;

        virtual void on_update_time(int new_time) override;
    protected:
        void generate_warrior(int time);
    };


    class weapon {
    public:
        weapon(const char* name): name(name) {}
        virtual ~weapon() = default;

        const char* name;
    };

    class sword: public weapon {
    public:
        sword(): weapon("sword") {}
        virtual ~sword() = default;
    };

    class bomb: public weapon {
    public:
        bomb(): weapon("bomb") {}
        virtual ~bomb() = default;
    };

    class arrow: public weapon {
    public:
        arrow(): weapon("arrow") {}
        virtual ~arrow() = default;
    };

    std::unique_ptr<weapon> get_weapon(int index)
    {
        switch (index) {
            case 0:
                return std::make_unique<sword>();
            case 1:
                return std::make_unique<bomb>();
            case 2:
                return std::make_unique<arrow>();
            default:
                return nullptr;
        }
    }


    class warrior {
    protected:
        camp_label _camp;
        int _health_point;
        int _id;

        std::vector<std::unique_ptr<weapon>> _weapons;
    public:
        warrior(camp_label camp, int health_point, int id, const char* name, int weapon_num = 0) noexcept
            : _camp(camp), _health_point(health_point), _id(id), name(name), _weapons(weapon_num)
        {
        }
        virtual ~warrior() = default;

        const char* name;
        camp_label camp() const noexcept { return _camp; }
        int health_point() const noexcept { return _health_point; }
        int id() const noexcept { return _id; }

        virtual void show_additional_information() const noexcept {};
    };

    class dragon: public warrior {
    private:
        double _morale;
    public:
        dragon(camp_label camp, int health_point, int id, double morale) noexcept;
        virtual ~dragon() = default;

        virtual void show_additional_information() const noexcept override;
    };

    class ninja: public warrior {
    public:
        ninja(camp_label camp, int health_point, int id) noexcept;
        virtual ~ninja() = default;

        virtual void show_additional_information() const noexcept override;
    };

    class iceman: public warrior {
    public:
        iceman(camp_label camp, int health_point, int id) noexcept;
        virtual ~iceman() = default;

        virtual void show_additional_information() const noexcept override;
    };

    class lion: public warrior {
    private:
        int _loyalty;
    public:
        lion(camp_label camp, int health_point, int id, int loyalty) noexcept
            : _loyalty(loyalty), warrior(camp, health_point, id, "lion")
        {
        }
        virtual ~lion() = default;

        virtual void show_additional_information() const noexcept override;
    };

    class wolf: public warrior {
    public:
        wolf(camp_label camp, int health_point, int id) noexcept
            : warrior(camp, health_point, id, "wolf")
        {
        }
        virtual ~wolf() = default;
    };

    std::unique_ptr<warrior> get_warrior(int index, camp_label camp, int health_point, int id, int left_hp)
    {
        switch (index) {
            case 0:
                return std::make_unique<dragon>(camp, health_point, id, static_cast<double>(left_hp) / health_point);
            case 1:
                return std::make_unique<ninja>(camp, health_point, id);
            case 2:
                return std::make_unique<iceman>(camp, health_point, id);
            case 3:
                return std::make_unique<lion>(camp, health_point, id, left_hp);
            case 4:
                return std::make_unique<wolf>(camp, health_point, id);
            default:
                return nullptr;
        }
    }

    class game_controller {
    private:
        const std::array<int, 5> _warrior_HP;

        std::vector<std::unique_ptr<city>> _citys;

        int _left_generating = 2;
    public:
        game_controller(int base_HP, int city_count, const std::array<int, 5>& warrior_HP);

        int warrior_HP(int n) const { return _warrior_HP[n]; }

        void send_message(game_message msg, std::any param = {});
        void run();
    };

    void headquarter::on_update_time(int new_time)
    {
        switch (minute(new_time)) {
            case 0:
                generate_warrior(new_time);
                break;
        }
        city::on_update_time(new_time);
    }

    void headquarter::generate_warrior(int time)
    {
        if (_stopped) return;
        static constexpr int red_order[] = { 2, 3, 4, 1, 0 },
            blue_order[] = { 3, 0, 1, 2, 4 };
        const auto& order = (_camp == camp_label::red) ? red_order : blue_order;
        int delta_warrior_type = 1;
        for (; delta_warrior_type <= 5; ++delta_warrior_type)
            if (_health_point >= _controller->warrior_HP(order[(_last_generate_warrior + delta_warrior_type) % 5]))
                break;
        if (delta_warrior_type == 6) {
            std::cout << std::setw(3) << std::setfill('0') << hour(time) << ' '
                << camp_name(_camp) << " headquarter stops making warriors" << std::endl;
            _controller->send_message(game_message::stop_generate_warrior);
            _stopped = true;
            return;
        }
        _last_generate_warrior = (_last_generate_warrior + delta_warrior_type) % 5;
        int index = order[_last_generate_warrior];
        int id = _warriors.size() + 1, hp = _controller->warrior_HP(index);
        ++_warrior_record[index];
        _health_point -= hp;
        _warriors.emplace_back(get_warrior(index, _camp, hp, id, _health_point));
        const auto name = _warriors.back()->name;
        std::cout << std::setw(3) << std::setfill('0') << hour(time) << ' '
            << camp_name(_camp) << ' ' << name << ' '
            << id << " born with strength " << hp << ','
            << _warrior_record[index] << ' ' << name << " in "
            << camp_name(_camp) << " headquarter" << std::endl;
        _warriors.back()->show_additional_information();
    }

    game_controller::game_controller(int base_HP, int city_count, const std::array<int, 5>& warrior_HP)
        : _warrior_HP(warrior_HP),
        _citys(city_count + 2)
    {
        _citys.front() = std::make_unique<headquarter>(camp_label::red, base_HP, this);
        _citys.back() = std::make_unique<headquarter>(camp_label::blue, base_HP, this);
        for (int i = 1; i <= city_count; ++i)
            _citys[i] = std::make_unique<city>();
    }

    void game_controller::send_message(game_message msg, std::any param)
    {
        switch (msg) {
            case game_message::stop_generate_warrior:
                --_left_generating;
                break;
            default:
                break;
        }
    }

    void game_controller::run()
    {
        int time = 0;
        for (; _left_generating > 0; time += 60) {
            for (auto& city : _citys) {
                city->on_update_time(time);
            }
        }
    }

    dragon::dragon(camp_label camp, int health_point, int id, double morale) noexcept
        : _morale(morale), warrior(camp, health_point, id, "dragon", 1)
    {
        _weapons[0] = get_weapon(id % 3);
    }

    void dragon::show_additional_information() const noexcept
    {
        std::cout << "It has a " << _weapons[0]->name
            << ",and it's morale is "
            << std::fixed << std::setprecision(2) << std::round(_morale * 100) / 100
            << std::endl;
    }

    ninja::ninja(camp_label camp, int health_point, int id) noexcept
        : warrior(camp, health_point, id, "ninja", 2)
    {
        _weapons[0] = get_weapon(id % 3);
        _weapons[1] = get_weapon((id + 1) % 3);
    }

    void ninja::show_additional_information() const noexcept
    {
        std::cout << "It has a " << _weapons[0]->name
            << " and a " << _weapons[1]->name
            << std::endl;
    }

    iceman::iceman(camp_label camp, int health_point, int id) noexcept
        : warrior(camp, health_point, id, "iceman", 1)
    {
        _weapons[0] = get_weapon(id % 3);
    }

    void iceman::show_additional_information() const noexcept
    {
        std::cout << "It has a " << _weapons[0]->name << std::endl;
    }

    void lion::show_additional_information() const noexcept
    {
        std::cout << "It's loyalty is " << _loyalty << std::endl;
    }
}

int main()
{
    int M;
    std::cin >> M;
    for (int i = 1; i <= M; ++i) {
        std::cout << "Case:" << i << std::endl;
        int hp;
        std::array<int, 5> hp_warrior;
        std::cin >> hp
            >> hp_warrior[0]
            >> hp_warrior[1]
            >> hp_warrior[2]
            >> hp_warrior[3]
            >> hp_warrior[4];
        warcraft::game_controller controller(hp, 0, hp_warrior);
        controller.run();
    }
    return 0;
}
