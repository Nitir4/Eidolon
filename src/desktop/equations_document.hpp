#pragma once
#include <tora/equations/equations.hpp>
#include <tora/core/number_formatter.hpp>
#include <string_view>
namespace tora::desktop {
class EquationsDocument {
public:
 static EquationsDocument blank(int variables,std::string title={},core::NumberFormat format={},bool matrixOnly=false);
 static EquationsDocument decode(std::string_view bytes);
 std::string encode() const;
 bool matrixOnly() const {return matrixOnly_;}
 const core::Matrix& augmented() const {return augmented_;}
 core::Matrix coefficients() const;
 std::vector<double> rhs() const;
 const std::string& title() const {return title_;}
 core::NumberFormat format() const {return format_;}
 const std::string& cell(int row,int column) const;
 void setCell(int row,int column,std::string text);
 void setTitle(std::string title);
 void setFormat(core::NumberFormat format);
private:
 bool matrixOnly_=false;
 core::Matrix augmented_;std::string title_,original_;core::NumberFormat format_;
 std::vector<std::string> cells_;
};
}
