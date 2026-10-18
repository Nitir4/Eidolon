#pragma once
#include <tora/queuing/queuing.hpp>
#include <tora/core/number_formatter.hpp>
#include <array>
#include <string_view>
namespace tora::desktop {
class QueuingDocument {
public:
 static QueuingDocument blank(std::string title={},core::NumberFormat format={});
 static QueuingDocument decode(std::string_view bytes);
 std::string encode()const;
 const std::string& title()const{return title_;}void setTitle(std::string title);
 core::NumberFormat format()const{return format_;}void setFormat(core::NumberFormat format);
 std::size_t count()const{return rows_.size();}
 const std::string& cell(std::size_t row,std::size_t column)const;
 void setCell(std::size_t row,std::size_t column,std::string value);
 using Scenario = std::array<std::string,5>;
 Scenario scenario(std::size_t row)const;
 void pasteScenario(std::size_t row,const Scenario& values);
 void insertScenario(std::size_t position);
 void addScenario();void deleteScenario(std::size_t row);
 queuing::Model model(std::size_t row)const;
private:
 std::string title_,original_;core::NumberFormat format_;
 std::vector<std::array<std::string,5>> rows_;
};
}
