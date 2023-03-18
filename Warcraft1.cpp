#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <any>
#include <iomanip>

namespace warcraft {
    enum class camp_label : int8_t {
        red = 0,
        blue = 1
    };

    enum class game_message : int8_t {
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

    class city : public game_object {
    public:
        virtual ~city() = default;
    };

    constexpr int warrior_type_count = 5;

    class headquarter : public city {
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
            : _camp(camp), _health_point(health_point), _controller(controller) {}
        virtual ~headquarter() = default;

        virtual void on_update_time(int new_time) override;
    protected:
        void generate_warrior(int time);
    };

    class warrior {
    protected:
        camp_label _camp;
        int _health_point;
        int _id;
    public:
        warrior(camp_label camp, int health_point, int id) noexcept
            : _camp(camp), _health_point(health_point), _id(id) {}
        virtual ~warrior() = default;
    };

    class iceman : public warrior {
    public:
        using warrior::warrior;
        virtual ~iceman() = default;
    };

    class lion : public warrior {
    public:
        using warrior::warrior;
        virtual ~lion() = default;
    };

    class wolf : public warrior {
    public:
        using warrior::warrior;
        virtual ~wolf() = default;
    };

    class ninja : public warrior {
    public:
        using warrior::warrior;
        virtual ~ninja() = default;
    };

    class dragon : public warrior {
    public:
        using warrior::warrior;
        virtual ~dragon() = default;
    };

    constexpr char warrior_name[warrior_type_count][7] = { "dragon", "ninja", "iceman", "lion", "wolf" };
    std::unique_ptr<warrior> get_warrior(int index, camp_label camp, int health_point, int id)
    {
        switch (index) {
        case 0:
            return std::make_unique<dragon>(camp, health_point, id);
        case 1:
            return std::make_unique<ninja>(camp, health_point, id);
        case 2:
            return std::make_unique<iceman>(camp, health_point, id);
        case 3:
            return std::make_unique<lion>(camp, health_point, id);
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
        const auto& name = warrior_name[index];
        int id = _warriors.size() + 1, hp = _controller->warrior_HP(index);
        ++_warrior_record[index];
        _health_point -= hp;
        _warriors.emplace_back(get_warrior(index, _camp, hp, id));
        std::cout << std::setw(3) << std::setfill('0') << hour(time) << ' '
            << camp_name(_camp) << ' ' << name << ' '
            << id << " born with strength " << hp << ','
            << _warrior_record[index] << ' ' << name << " in "
            << camp_name(_camp) << " headquarter" << std::endl;
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