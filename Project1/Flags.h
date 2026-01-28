#ifndef FLAGS_H
#define FLAGS_H

#include <memory>

class Flags
{
public:

	Flags();

	~Flags();

	static Flags& GetInstance();

	bool isDebug = false;

	bool isTimeStop = false;
private:
	/// <summary>
    /// 全般の状態を示すためのクラスのシングルトンのインスタンス
    /// </summary>
	static std::unique_ptr<Flags> instance;
};

#endif


