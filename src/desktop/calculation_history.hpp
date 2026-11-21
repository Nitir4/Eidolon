#pragma once
#include "assignment_controller.hpp"
#include "games_document.hpp"
namespace tora::desktop {
QString assignmentStepText(const AssignmentData& data,const assignment::Result& result,int index);
QString gameStepText(const GamesDocument& data,const games::Result& result,int index);
}
