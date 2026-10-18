#pragma once
#include <tora/planning/planning.hpp>
#include <tora/core/number_formatter.hpp>
#include <string>
#include <string_view>
namespace tora::desktop {
// Original 8001 CPM / 8002 PERT record stream. Unedited imports retain all bytes.
class PlanningDocument {
public:
 static PlanningDocument create(std::string title,planning::Problem problem,core::NumberFormat format={});
 static PlanningDocument decode(std::string_view bytes);
 std::string encode()const;
 const std::string& title()const{return title_;}
 const planning::Problem& problem()const{return problem_;}
 core::NumberFormat format()const{return format_;}
 void update(std::string title,planning::Problem problem,core::NumberFormat format);
private:
 std::string title_,original_;planning::Problem problem_;core::NumberFormat format_;
};
}
