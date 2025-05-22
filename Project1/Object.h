#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <sstream>

class Object : public std::enable_shared_from_this<Object>
{
public:
    Object(const std::string& name);

	virtual ~Object();

    /// <summary> 
    /// 子供を追加する
    /// </summary>
    /// <param name="obj">追加する子供オブジェクト</param>
	void AddChild(std::shared_ptr<Object> obj);

	/// <summary>
    /// 子供のシェアドポインタの配列を返す
    /// </summary>
    /// <returns>リスト</returns>
	std::vector<std::shared_ptr<Object>> GetChild();

	/// <summary>
    /// 指定した名前の子供のシェアドポインタを返す
    /// </summary>
    /// <returns>子供のポインタ</returns>
	std::shared_ptr<Object> GetChild(std::string name);

	/// <summary>
    /// 指定したインデックスの子供のシェアドポインタを返す
    /// </summary>
    /// <returns>子供のポインタ</returns>
	std::shared_ptr<Object> GetChild(int index);

	/// <summary>
    /// 親をセットする
    /// </summary>
    /// <returns>セットする親</returns>
	void SetParent(std::shared_ptr<Object> obj);

	void SetName(std::string name);

	std::string GetName();

	/// <summary>
    /// 親のシェアドポインタを返す
    /// </summary>
    /// <returns>親のシェアドポインタ</returns>
	 std::shared_ptr<Object> GetParent();

	 /// <summary>
	 /// 親が存在するかどうかを返す
	 /// </summary>
	 bool HasParent();

	 /// <summary>
     /// 子供が存在するかどうかを返す
     /// </summary>
	 bool HasChild();
private:
	//子供のオブジェクトの辞書型配列
	std::vector<std::shared_ptr<Object>> objsChild;

	//親のオブジェクトのポインター
	std::shared_ptr<Object> parent = nullptr;

	/// <summary>
    /// オブジェクトの名前
    /// </summary>
	std::string name;
};

#endif