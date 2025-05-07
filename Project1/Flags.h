#ifndef FLAGS_H
#define FLAGS_H

#include <memory>

class Flags
{
public:

	Flags();

	~Flags();

	static Flags& GetInstance();

	std::shared_ptr<bool> isDebug = std::make_shared<bool>();

	std::shared_ptr<bool> isTimeStop = std::make_shared<bool>();
private:
	/// <summary>
    /// 全般の状態を示すためのクラスのシングルトンのインスタンス
    /// </summary>
	static std::unique_ptr<Flags> instance;
};

#endif


