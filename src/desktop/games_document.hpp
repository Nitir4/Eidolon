#pragma once
#include <tora/games/games.hpp>
#include <tora/core/number_formatter.hpp>
#include <string_view>
namespace tora::desktop {
class GamesDocument {
public:
 static GamesDocument blank(int rows,int columns,std::string title={},core::NumberFormat format={});
 static GamesDocument decode(std::string_view bytes);
 std::string encode() const;
 const core::Matrix& payoff() const {return payoff_;}
 const std::string& title() const {return title_;}
 core::NumberFormat format() const {return format_;}
 const std::string& cell(int row,int column) const;
 void setCell(int row,int column,std::string text);
 void setTitle(std::string title);
 void setFormat(core::NumberFormat format);
 void insert(bool row,int position);void erase(bool row,int position);
 std::vector<std::string> line(bool row,int position)const;
 void paste(bool row,int position,const std::vector<std::string>& values);
private:
 core::Matrix payoff_;std::string title_,original_;core::NumberFormat format_;
 std::vector<std::string> cells_;
};
}
