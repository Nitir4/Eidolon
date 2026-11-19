#pragma once
#include <tora/lp/problem.hpp>
#include <stdexcept>

inline tora::lp::LpProblem sensitivityCase(char id)
{
    using namespace tora::lp;
    auto p=LpProblem::blank(2,id=='A' || id=='C'?3:2);
    p.title=std::string("Sensitivity ")+id;
    p.sense=id=='B' || id=='D'?ObjectiveSense::Minimize:ObjectiveSense::Maximize;
    p.objective=id=='A'?std::vector<double>{1,1}:id=='C'?std::vector<double>{-1,2}:std::vector<double>{1,2};
    p.variables[0].unrestricted=id=='C' || id=='D';
    switch(id) {
    case 'A':p.constraints={{{-1,-1},Relation::LessEqual,-2},{{1,1},Relation::LessEqual,4},{{1,0},Relation::LessEqual,3}};break;
    case 'B':p.constraints={{{-1,0},Relation::LessEqual,-1},{{1,1},Relation::GreaterEqual,3}};break;
    case 'C':p.constraints={{{1,1},Relation::LessEqual,2},{{-1,1},Relation::LessEqual,4},{{0,1},Relation::LessEqual,3}};break;
    case 'D':p.constraints={{{1,1},Relation::Equal,-1},{{0,1},Relation::LessEqual,3}};break;
    default:throw std::invalid_argument("Unknown sensitivity case");
    }
    return p;
}
