/*********************************************************
*  23�� �������ʵϰ ħ���������ҵ							
*  ��Ŀ��http://cxsjsx.openjudge.cn/hw202306/E/
*  
*  ����ܹ���
*  game_object(������)��������Ϸ�еĶ���Ĺ�ͬ����
*  city, warrior(������), weapon(������)��������ֱ������
*  �����໹�и��Ե����࣬��ʾ������Ķ���
*  
*  game_controller->city->warrior->weapon
*  ��ͷ��ߵ�������ұߵ����unique pointer
*  ����ʱ��ֻ��������һ��game_controller�������(����ģʽ)
*  ��Ϸ��game_controller::run()����
*  run()��ִ��ʵ�ʶ�����ֻ����city����ʱ����µ���Ϣ
*  ʱ����µ���Ϣ��������Ķ���������
*  ÿ������ͨ��on_update_time���������ض�ʱ��ʱ�����Լ��Ķ���
*  
*  ��ʿǰ������Ϸֹͣ��controllerִ��
*  ��������ʿ��ս�������ڵ�cityִ��
*  ����������Ч����ս��˫��warriorִ��
*********************************************************/

#include <iostream>
#include <vector>
#include <array>
#include <stdexcept>
#include <any>
#include <iomanip>
#include <sstream>
#include <math.h>
#include <algorithm>

namespace warcraft
{
	constexpr int camp_count = 2;
	constexpr int warrior_type_count = 5;
	constexpr int weapon_type_count = 3;

	enum class camp_label : int8_t {
		red = 0,
		blue = 1
	};

	enum class game_message : int8_t {
		game_over
	};

	std::string camp_name(camp_label camp) noexcept
	{
		switch (camp) {
		case camp_label::red:
			return "red";
		case camp_label::blue:
			return "blue";
		}
	}

	int camp_num(camp_label camp) noexcept
	{
		switch (camp) {
		case camp_label::red:
			return 0;
		case camp_label::blue:
			return 1;
		}
	}

	const std::string weapon_name(int index) noexcept
	{
		switch (index) {
		case 0:
			return "sword";
		case 1:
			return "bomb";
		case 2:
			return "arrow";
		}
	}

	camp_label enemy_camp(camp_label camp) noexcept
	{
		switch (camp) {
		case camp_label::red:
			return camp_label::blue;
		case camp_label::blue:
			return camp_label::red;
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

	// ʱ���ʽ��hhh:mm
	std::string time_to_str(int time) noexcept
	{
		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(3) << hour(time)
			<< ':'
			<< std::setw(2) << minute(time);
		return oss.str();
	}

	// ������Ϸ����Ĺ�ͬ���࣬������
	class game_object {
	public:
		game_object(const std::string& name) noexcept : name(name) {}
		virtual ~game_object() = 0;
		virtual void on_update_time(int new_time) {}

		const std::string name;
	};

	class city;
	class warrior;
	class headquarter;

	class game_controller {
	private:
		// ����ģʽ
		inline static game_controller* _the_controller;

		// front()�Ǻ�ħ��˾� back()����ħ��˾� ������
		// _city[i]�Ǳ��Ϊi�ĳ���
		std::vector<std::unique_ptr<city>> _citys;

		bool _game_over = false;
	public:
		const int lion_loyalty_reduce;
		const int end_time;
		const std::array<int, warrior_type_count> warrior_HP, warrior_force;

		game_controller(int base_HP, int city_count, int loyalty_reduce, int end_time,
			const std::array<int, warrior_type_count>& warrior_HP, const std::array<int, warrior_type_count>& warrior_force);
		~game_controller();

		// ��ȡ��ǰȫ��controller����
		static game_controller& get_controller() { return *_the_controller; }

		headquarter& get_headquarter(camp_label camp);

		void send_message(game_message msg, std::any param = {});
		void on_update_time(int new_time);

		// ��ʿǰ��
		void warrior_move_forward(int time);

		// ��Ϸ����
		void run();
	};

	class city : public game_object {
	protected:
		int _city_id;

		std::array<std::unique_ptr<warrior>, camp_count> _warriors;
		// ����������ʿ��ʱ�������������ÿ��Сʱ�����һ���ͷſռ�
		std::vector<std::unique_ptr<warrior>> _warrior_to_clean;
	public:
		city(int id) noexcept;
		city(int id, const std::string& name) noexcept;
		virtual ~city() = default;

		// ��ȡ������ĳһ����Ӫ����ʿ
		std::unique_ptr<warrior>& warrior_of(camp_label camp) noexcept { return _warriors[camp_num(camp)]; }
		const std::unique_ptr<warrior>& warrior_of(camp_label camp) const noexcept { return _warriors[camp_num(camp)]; }

		// ����ʿ�����ɾ������
		void remove_warrior(camp_label camp);
		virtual void on_warrior_march_to(int time) noexcept;
		void fight(int time) noexcept;

		virtual void on_update_time(int new_time) override;
	};

	class headquarter : public city {
	protected:
		camp_label _camp;
		int _health_point;

		// ��һ�����ɵ���ʿ���
		int _last_generate_warrior = -1;
		// ���ɹ�����ʿ����
		int _generated_count = 0;
		// �Ƿ��Ѿ�ֹͣ������ʿ
		bool _stopped = false;
	public:
		headquarter(camp_label camp, int health_point, int id) noexcept;
		virtual ~headquarter() = default;

		bool isoccupied() const noexcept { return warrior_of(enemy_camp(_camp)).operator bool(); }

		virtual void on_warrior_march_to(int time) noexcept override;
		void show_health_point(int time) const noexcept;

		virtual void on_update_time(int new_time) override;
	protected:
		void generate_warrior(int time);
	};

	class weapon : public game_object {
	protected:
		int _durability, _force = 0;
	public:
		weapon(const std::string& name, int index, int durability);
		virtual ~weapon() = 0;

		const int weapon_index;
		
		int durability() const noexcept { return _durability; }
		int force() const noexcept { return _force; }
		// return: �ܷ����ʹ������
		virtual bool reduce_durability() noexcept { return --_durability > 0; }
		// ������������������ʿ�Ĺ�����
		virtual int set_force(int holder_force) noexcept = 0;
	};

	class sword : public weapon {
	public:
		// sword�;�-1��ʾ�����;�
		sword() : weapon("sword", 0, -1) {}
		virtual ~sword() = default;

		virtual bool reduce_durability() noexcept override { return true; }
		virtual int set_force(int holder_force) noexcept override { return _force = holder_force * 2 / 10; }
	};

	class bomb : public weapon {
	public:
		bomb() noexcept : weapon("bomb", 1, 1) {}
		virtual ~bomb() = default;

		virtual int set_force(int holder_force) noexcept override { return _force = holder_force * 4 / 10; }
	};

	class arrow : public weapon {
	public:
		arrow() noexcept : weapon("arrow", 2, 2) {}
		virtual ~arrow() = default;

		virtual int set_force(int holder_force) noexcept override { return _force = holder_force * 3 / 10; }
	};

	class warrior : public game_object {
		friend class game_controller; // ���ĳ���
		friend class wolf; // ��������
		friend class city; // ս��
	protected:
		camp_label _camp;
		int _health_point, _force;
		int _id;

		std::vector<std::unique_ptr<weapon>> _weapons;

		city* _city;
	public:
		warrior(camp_label camp, int health_point, int force, int id, const std::string& type_name, int weapon_num = 0) noexcept;
		virtual ~warrior() = 0;

		const std::string type_name;
		constexpr static int max_weapon_count = 10;
		camp_label camp() const noexcept { return _camp; }
		int health_point() const noexcept { return _health_point; }
		int force() const noexcept { return _force; }
		int id() const noexcept { return _id; }
		int weapon_count() const noexcept { return _weapons.size(); }
		weapon& weapon_at(int index) noexcept { return *_weapons[index]; }

		// ���Լ���ͬһ�����ڵĵз���ʿ������ֻ���Լ�����nullptr
		warrior* enemy_now() const noexcept { return _city->warrior_of(enemy_camp(_camp)).get(); }

		virtual void show_additional_information() const noexcept {}
		virtual void on_move_forward() noexcept {}
		virtual void show_weapon(int time) noexcept;
		// ս��ǰ׼������
		virtual void prefight() noexcept;
		// ս����������
		virtual void postfight() noexcept;
		// ս���н��н���
		virtual void on_attacking(weapon& weapon, warrior& aim) noexcept;
		// ս���б�����
		virtual void on_attacked(weapon& weapon, warrior& attacker) noexcept;
		// ս������
		virtual void on_alive(int time) noexcept {};

		virtual void on_update_time(int new_time) override;
	};

	class dragon : public warrior {
	private:
		double _morale;
	public:
		dragon(camp_label camp, int health_point, int force, int id, double morale) noexcept;
		virtual ~dragon() = default;

		virtual void on_alive(int time) noexcept override;
	};

	class ninja : public warrior {
	public:
		ninja(camp_label camp, int health_point, int force, int id) noexcept;
		virtual ~ninja() = default;

		virtual void on_attacking(weapon& weapon, warrior& aim) noexcept override;
	};

	class iceman : public warrior {
	public:
		iceman(camp_label camp, int health_point, int force, int id) noexcept;
		virtual ~iceman() = default;

		virtual void on_move_forward() noexcept override { _health_point -= _health_point / 10; }
	};

	class lion : public warrior {
	private:
		int _loyalty;
	public:
		lion(camp_label camp, int health_point, int force, int id, int loyalty) noexcept;
		virtual ~lion() = default;

		virtual void show_additional_information() const noexcept override;
		virtual void on_move_forward() noexcept override { _loyalty -= game_controller::get_controller().lion_loyalty_reduce; }
		void try_runaway(int new_time) noexcept;

		virtual void on_update_time(int new_time) override;
	};

	class wolf : public warrior {
	public:
		wolf(camp_label camp, int health_point, int force, int id) noexcept;
		virtual ~wolf() = default;

		void snatch(int time) noexcept;

		virtual void on_update_time(int new_time) override;
	};

	game_object::~game_object() = default;

	void headquarter::on_update_time(int new_time)
	{
		switch (minute(new_time)) {
		case 0:
			generate_warrior(new_time);
			break;
		case 50:
			show_health_point(new_time);
			break;
		}
		city::on_update_time(new_time);
	}

	headquarter::headquarter(camp_label camp, int health_point, int id) noexcept
		: _camp(camp), _health_point(health_point),
		city(id, camp_name(camp) + " headquarter")
	{}

	std::unique_ptr<warrior> make_warrior(int index, camp_label camp, int health_point, int force, int id, int left_hp)
	{
		switch (index) {
		case 0:
			return std::make_unique<dragon>(camp, health_point, force, id, static_cast<double>(left_hp) / health_point);
		case 1:
			return std::make_unique<ninja>(camp, health_point, force, id);
		case 2:
			return std::make_unique<iceman>(camp, health_point, force, id);
		case 3:
			return std::make_unique<lion>(camp, health_point, force, id, left_hp);
		case 4:
			return std::make_unique<wolf>(camp, health_point, force, id);
		default:
			return nullptr;
		}
	}

	void headquarter::generate_warrior(int time)
	{
		if (_stopped) return;

		auto& controller = game_controller::get_controller();
		// �̶�������˳��
		static constexpr int generate_order[camp_count][5]
			= { { 2, 3, 4, 1, 0 }, { 3, 0, 1, 2, 4 } };
		const auto& order = generate_order[camp_num(_camp)];
		if (_health_point < controller.warrior_HP[order[(_last_generate_warrior + 1) % warrior_type_count]]) {
			_stopped = true;
			return;
		}
		_last_generate_warrior = (_last_generate_warrior + 1) % warrior_type_count;
		int index = order[_last_generate_warrior];
		int id = ++_generated_count, hp = controller.warrior_HP[index], force = controller.warrior_force[index];
		_health_point -= hp;
		warrior_of(_camp) = make_warrior(index, _camp, hp, force, id, _health_point);
		std::cout << time_to_str(time) << ' '
			<< warrior_of(_camp)->name << " born" << std::endl;
		warrior_of(_camp)->show_additional_information();
	}

	void headquarter::on_warrior_march_to(int time) noexcept
	{
		if (auto& warrior = warrior_of(enemy_camp(_camp)); warrior) {
			warrior->on_move_forward();
			std::cout << time_to_str(time) << ' '
				<< warrior->name << " reached "
				<< name << " with "
				<< warrior->health_point() << " elements and force "
				<< warrior->force() << std::endl;
			std::cout << time_to_str(time) << ' '
				<< name << " was taken" << std::endl;
		}
	}

	void headquarter::show_health_point(int time) const noexcept
	{
		std::cout << time_to_str(time) << ' '
			<< _health_point << " elements in "
			<< name << std::endl;
	}

	game_controller::game_controller(int base_HP, int city_count, int loyalty_reduce, int end_time,
		const std::array<int, warrior_type_count>& warrior_HP, const std::array<int, warrior_type_count>& warrior_force)
		: warrior_HP(warrior_HP),
		warrior_force(warrior_force),
		_citys(city_count + 2),
		lion_loyalty_reduce(loyalty_reduce),
		end_time(end_time)
	{
		// ����ģʽ
		if (_the_controller)
			throw std::runtime_error("One controller has been existing!");
		_the_controller = this;
		_citys.front() = std::make_unique<headquarter>(camp_label::red, base_HP, 0);
		_citys.back() = std::make_unique<headquarter>(camp_label::blue, base_HP, city_count + 1);
		for (int i = 1; i <= city_count; ++i)
			_citys[i] = std::make_unique<city>(i);
	}

	game_controller::~game_controller()
	{
		// �ÿ�ȫ�ֱ���
		_the_controller = nullptr;
	}

	headquarter& game_controller::get_headquarter(camp_label camp)
	{
		switch (camp) {
		case camp_label::red:
			return dynamic_cast<headquarter&>(*_citys.front());
		case camp_label::blue:
			return dynamic_cast<headquarter&>(*_citys.back());
		}
	}

	void game_controller::send_message(game_message msg, std::any param)
	{
		switch (msg) {
		case game_message::game_over:
			_game_over = true;
			break;
		}
	}

	void game_controller::run()
	{
		int time = 0;
		for (; time <= end_time and !_game_over; ++time) {
			// updatetime��㴫����
			// controller->city->warrior->weapon
			// game_objectӦ��дon_update_time�����ض�ʱ�����һ������
			on_update_time(time);
			for (auto& city : _citys) {
				city->on_update_time(time);
			}
		}
	}

	void game_controller::on_update_time(int new_time)
	{
		switch (minute(new_time)) {
		case 10:
			warrior_move_forward(new_time);
			break;
		}
	}

	void game_controller::warrior_move_forward([[maybe_unused]] int time)
	{
		int city_index = 0;
		// ����ʿ����С�ĳ����ƶ�
		for (; city_index < _citys.size() - 1; ++city_index) {
			_citys[city_index]->warrior_of(camp_label::blue) = move(_citys[city_index + 1]->warrior_of(camp_label::blue));
			if (_citys[city_index]->warrior_of(camp_label::blue))
				_citys[city_index]->warrior_of(camp_label::blue)->_city = _citys[city_index].get();
		}
		// ����ʿ���Ŵ�ĳ����ƶ�
		for (; city_index > 0; --city_index) {
			_citys[city_index]->warrior_of(camp_label::red) = move(_citys[city_index - 1]->warrior_of(camp_label::red));
			if (_citys[city_index]->warrior_of(camp_label::red))
				_citys[city_index]->warrior_of(camp_label::red)->_city = _citys[city_index].get();
		}
		// �Ƿ�������һ��˾���ռ��
		if (get_headquarter(camp_label::red).warrior_of(camp_label::blue) or
			get_headquarter(camp_label::blue).warrior_of(camp_label::red))
			send_message(game_message::game_over);
	}

	weapon::weapon(const std::string& name, int index, int durability) :
		weapon_index(index), _durability(durability), game_object(name)
	{}

	weapon::~weapon() = default;

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

	// ������ʹ�õ�˳��
	bool use_cmp(const weapon& weapon1, const weapon& weapon2)
	{
		return (weapon1.weapon_index < weapon2.weapon_index)
			or (weapon1.weapon_index == weapon2.weapon_index and
				weapon1.durability() < weapon2.durability());
	}

	bool use_cmp_uptr(const std::unique_ptr<weapon>& weapon1, const std::unique_ptr<weapon>& weapon2)
	{
		return use_cmp(*weapon1, *weapon2);
	}

	// ����������ͽɻ��˳��
	bool snatch_cmp(const weapon& weapon1, const weapon& weapon2)
	{
		return (weapon1.weapon_index < weapon2.weapon_index)
			or (weapon1.weapon_index == weapon2.weapon_index and
				weapon1.durability() > weapon2.durability());
	}

	bool snatch_cmp_uptr(const std::unique_ptr<weapon>& weapon1, const std::unique_ptr<weapon>& weapon2)
	{
		return snatch_cmp(*weapon1, *weapon2);
	}

	// ͬ���ͬ�;õ�������Ϊ����ȵ�
	bool operator==(const weapon& weapon1, const weapon& weapon2)
	{
		return weapon1.weapon_index == weapon2.weapon_index
			and weapon1.durability() == weapon2.durability();
	}

	bool operator==(const std::unique_ptr<weapon>& weapon1, const std::unique_ptr<weapon>& weapon2)
	{
		return *weapon1 == *weapon2;
	}

	warrior::warrior(camp_label camp, int health_point, int force, int id, const std::string& type_name, int weapon_num) noexcept
		: _camp(camp), _health_point(health_point), _force(force),
		_id(id), type_name(type_name),
		_city(&game_controller::get_controller().get_headquarter(camp)),
		game_object(camp_name(camp) + ' ' + type_name + ' ' + std::to_string(id))
	{}

	warrior::~warrior() = default;

	void warrior::on_update_time(int new_time)
	{
		switch (minute(new_time)) {
		case 55:
			show_weapon(new_time);
			break;
		}
		for (auto& weapon : _weapons)
			weapon->on_update_time(new_time);
		game_object::on_update_time(new_time);
	}

	void warrior::show_weapon(int time) noexcept
	{
		std::array<int, weapon_type_count> count{ 0 };
		for (const auto& weapon : _weapons)
			++count[weapon->weapon_index];
		std::cout << time_to_str(time) << ' '
			<< name << " has ";
		for (int index = 0; index < weapon_type_count; ++index)
			std::cout << count[index] << ' ' << weapon_name(index) << ' ';
		std::cout << "and " << _health_point << " elements" << std::endl;
	}

	void warrior::prefight() noexcept
	{
		std::sort(_weapons.begin(), _weapons.end(), use_cmp_uptr);
	}

	void warrior::postfight() noexcept
	{
		auto iter = _weapons.cbegin();
		while (iter != _weapons.cend()) {
			if ((*iter)->durability() == 0)
				iter = _weapons.erase(iter);
			else
				++iter;
		}
	}

	void warrior::on_attacking(weapon& weapon, warrior& aim) noexcept
	{
		if (weapon.name == "bomb")
			_health_point -= weapon.force() / 2;
		weapon.reduce_durability();
	}

	void warrior::on_attacked(weapon& weapon, warrior& attacker) noexcept
	{
		_health_point -= weapon.force();
	}

	dragon::dragon(camp_label camp, int health_point, int force, int id, double morale) noexcept
		: _morale(morale), warrior(camp, health_point, force, id, "dragon", 1)
	{
		_weapons.emplace_back(get_weapon(id % weapon_type_count));
	}

	void dragon::on_alive(int time) noexcept
	{
		std::cout << time_to_str(time) << ' '
			<< name << " yelled in " << _city->name << std::endl;
	}

	ninja::ninja(camp_label camp, int health_point, int force, int id) noexcept
		: warrior(camp, health_point, force, id, "ninja", 2)
	{
		_weapons.emplace_back(get_weapon(id % weapon_type_count));
		_weapons.emplace_back(get_weapon((id + 1) % weapon_type_count));
	}

	void ninja::on_attacking(weapon& weapon, warrior& aim) noexcept
	{
		if (weapon.name == "bomb") {
			// ninjaʹ��bomb����ʹ�Լ�����
			int hp = _health_point;
			warrior::on_attacking(weapon, aim);
			_health_point = hp;
		}
		else {
			warrior::on_attacking(weapon, aim);
		}
	}

	iceman::iceman(camp_label camp, int health_point, int force, int id) noexcept
		: warrior(camp, health_point, force, id, "iceman", 1)
	{
		_weapons.emplace_back(get_weapon(id % weapon_type_count));
	}

	lion::lion(camp_label camp, int health_point, int force, int id, int loyalty) noexcept
		: _loyalty(loyalty), warrior(camp, health_point, force, id, "lion")
	{
		_weapons.emplace_back(get_weapon(id % weapon_type_count));
	}

	void lion::show_additional_information() const noexcept
	{
		std::cout << "Its loyalty is " << _loyalty << std::endl;
	}

	void lion::on_update_time(int new_time)
	{
		switch (minute(new_time)) {
		case 5:
			try_runaway(new_time);
			break;
		}
		warrior::on_update_time(new_time);
	}

	void lion::try_runaway(int time) noexcept
	{
		if (_loyalty <= 0) {
			std::cout << time_to_str(time) << ' '
				<< name << " ran away" << std::endl;

			// ����������б��ӳ�ɾ��
			_city->remove_warrior(_camp);
		}
	}

	wolf::wolf(camp_label camp, int health_point, int force, int id) noexcept
		: warrior(camp, health_point, force, id, "wolf")
	{}

	void wolf::on_update_time(int new_time)
	{
		switch (minute(new_time)) {
		case 35:
			snatch(new_time);
			break;
		}
		warrior::on_update_time(new_time);
	}

	void wolf::snatch(int time) noexcept
	{
		auto enemy = enemy_now();
		if (!enemy or enemy->type_name == "wolf" or enemy->_weapons.empty())
			return;

		std::sort(enemy->_weapons.begin(), enemy->_weapons.end(), snatch_cmp_uptr);
		auto iter = enemy->_weapons.cbegin();
		auto index = (*iter)->weapon_index;
		int capacity = max_weapon_count - _weapons.size();
		int snatch_num = 0;
		for (; snatch_num < capacity and
			iter != enemy->_weapons.cend() and (*iter++)->weapon_index == index;
			++snatch_num);

		std::cout << time_to_str(time) << ' '
			<< name << " took "
			<< snatch_num << ' ' << weapon_name(index)
			<< " from " << enemy->name
			<< " in " << _city->name << std::endl;

		std::move(enemy->_weapons.begin(), enemy->_weapons.begin() + snatch_num,
			std::back_inserter(_weapons));
		enemy->_weapons.erase(enemy->_weapons.begin(), enemy->_weapons.begin() + snatch_num);
	}

	city::city(int id) noexcept
		: city(id, "city " + std::to_string(id))
	{}

	city::city(int id, const std::string & name) noexcept
		: _city_id(id), game_object(name)
	{}

	void city::remove_warrior(camp_label camp)
	{
		if (warrior_of(camp))
			_warrior_to_clean.emplace_back(move(warrior_of(camp)));
	}

	void city::on_update_time(int new_time)
	{
		switch (minute(new_time)) {
		case 10:
			on_warrior_march_to(new_time);
			break;
		case 40:
			fight(new_time);
			break;
		case 59:
			// �ͷ���������ʿ�Ŀռ�
			_warrior_to_clean.clear();
			break;
		}
		for (auto& warrior : _warriors)
			if (warrior)
				warrior->on_update_time(new_time);
		game_object::on_update_time(new_time);
	}

	void city::on_warrior_march_to(int time) noexcept
	{
		auto show_march_info = [&](const warrior& w) {
			std::cout << time_to_str(time) << ' '
				<< w.name << " marched to "
				<< name << " with "
				<< w.health_point() << " elements and force "
				<< w.force() << std::endl;
		};
		if (warrior_of(camp_label::red)) {
			warrior_of(camp_label::red)->on_move_forward();
			show_march_info(*warrior_of(camp_label::red));
		}
		if (warrior_of(camp_label::blue)) {
			warrior_of(camp_label::blue)->on_move_forward();
			show_march_info(*warrior_of(camp_label::blue));
		}
	}

	void city::fight(int time) noexcept
	{
		for (const auto& warrior : _warriors)
			if (!warrior)
				return;
		for (const auto& warrior : _warriors)
			warrior->prefight();
		camp_label attacker_camp = (_city_id % 2 == 1 ? camp_label::red : camp_label::blue);
		bool end_fight[camp_count]{ false };
		bool end = false;
		int weapon_to_use[camp_count]{ 0 };
		while (!end) {
			auto& attacker = warrior_of(attacker_camp),
				& attacked = warrior_of(enemy_camp(attacker_camp));

			bool has_effective_weapon = false;
			for (int i = 0; i < attacker->weapon_count(); ++i) {
				attacker->weapon_at(i).set_force(attacker->force());
				if (attacker->weapon_at(i).durability() > 0 or
					attacker->weapon_at(i).force() > 0)
					has_effective_weapon = true;
			}
			// ��ѡ����
			int weapon_index = weapon_to_use[camp_num(attacker_camp)],
				delta_weapon_index = 0,
				weapon_count = attacker->weapon_count();
			for (; delta_weapon_index < weapon_count; ++delta_weapon_index)
				if (attacker->weapon_at((weapon_index + delta_weapon_index) % weapon_count).durability() != 0)
					break;
			if (!has_effective_weapon or delta_weapon_index == weapon_count) { // ������
				end_fight[camp_num(attacker_camp)] = true;
				end = end_fight[camp_num(enemy_camp(attacker_camp))];
			}
			else { // ������
				weapon_index = (weapon_index + delta_weapon_index) % weapon_count;
				auto& weapon_using = attacker->weapon_at(weapon_index);
				attacker->on_attacking(weapon_using, *attacked);
				attacked->on_attacked(weapon_using, *attacker);

				weapon_to_use[camp_num(attacker_camp)] = (weapon_index + 1) % weapon_count;

				// �ж��Ƿ�����
				end = attacker->health_point() <= 0 or attacked->health_point() <= 0;
			}
			// ����������
			attacker_camp = enemy_camp(attacker_camp);
		}
		if ((warrior_of(camp_label::red)->health_point() <= 0 and
			warrior_of(camp_label::blue)->health_point() <= 0)) {
			// ˫����ս��
			std::cout << time_to_str(time) << " both "
				<< warrior_of(camp_label::red)->name << " and "
				<< warrior_of(camp_label::blue)->name << " died in "
				<< name << std::endl;
			remove_warrior(camp_label::red);
			remove_warrior(camp_label::blue);
		}
		else if ((warrior_of(camp_label::red)->health_point() > 0 and
			warrior_of(camp_label::blue)->health_point() > 0)) {
			// ˫�������
			std::cout << time_to_str(time) << " both "
				<< warrior_of(camp_label::red)->name << " and "
				<< warrior_of(camp_label::blue)->name << " were alive in "
				<< name << std::endl;
			warrior_of(camp_label::red)->postfight();
			warrior_of(camp_label::red)->on_alive(time);
			warrior_of(camp_label::blue)->postfight();
			warrior_of(camp_label::blue)->on_alive(time);
		}
		else {
			// һ��սʤ��һ��
			auto& winner = (warrior_of(camp_label::red)->health_point() > 0 ? warrior_of(camp_label::red) : warrior_of(camp_label::blue));
			auto loser = winner->enemy_now();
			std::cout << time_to_str(time) << ' '
				<< winner->name << " killed " << loser->name
				<< " in " << name
				<< " remaining " << winner->health_point() << " elements"
				<< std::endl;

			// �ɻ�����
			loser->postfight();
			winner->postfight();

			std::sort(loser->_weapons.begin(), loser->_weapons.end(), snatch_cmp_uptr);
			int capacity = warrior::max_weapon_count - winner->_weapons.size();
			int snatch_num = std::min(capacity, loser->weapon_count());
			std::move(loser->_weapons.begin(), loser->_weapons.begin() + snatch_num,
				std::back_inserter(winner->_weapons));

			winner->on_alive(time);
			remove_warrior(loser->camp());
		}
	}
}

int main()
{
	int game_count;
	std::cin >> game_count;
	for (int game_index = 1; game_index <= game_count; ++game_index) {
		int hp_base, citys, loyalty, time;
		std::array<int, warcraft::warrior_type_count> hp, force;
		std::cin >> hp_base >> citys >> loyalty >> time;
		for (int i = 0; i < warcraft::warrior_type_count; ++i)
			std::cin >> hp[i];
		for (int i = 0; i < warcraft::warrior_type_count; ++i)
			std::cin >> force[i];
		warcraft::game_controller controller(hp_base, citys, loyalty, time, hp, force);
		
		std::cout << "Case " << game_index << ':' << std::endl; 
		controller.run();
	}
	return 0;
}
