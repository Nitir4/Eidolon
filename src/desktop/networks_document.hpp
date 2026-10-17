#pragma once
#include <tora/networks/networks.hpp>
#include <tora/core/number_formatter.hpp>
#include <string>
#include <string_view>
namespace tora::desktop {
enum class NetworkMode {MinimalTree,ShortestRoute,MaximalFlow};
class NetworksDocument {
public:
 static NetworksDocument blank(NetworkMode mode,int nodes,std::string title={},core::NumberFormat format={});
 // Marker 0 is accepted only in explicit ShortestRoute context.
 static NetworksDocument decode(std::string_view bytes,NetworkMode expectedMode);
 std::string encode()const;
 NetworkMode mode()const{return mode_;}int nodes()const{return nodes_;}
 const std::string& title()const{return title_;}core::NumberFormat format()const{return format_;}
 bool symmetrical()const{return symmetrical_;}
 const std::string& cell(int row,int column)const;
 void setCell(int row,int column,std::string value);
 void setTitle(std::string value);void setFormat(core::NumberFormat format);void setSymmetrical(bool enabled);
 void insertNode(int position);void eraseNode(int position);
 networks::Graph graph()const;
 std::string nodeName(int position)const;
private:
 int nodes_=0;NetworkMode mode_=NetworkMode::MinimalTree;bool symmetrical_=true;
 core::NumberFormat format_;std::string title_,original_;std::vector<std::string> cells_;
 std::size_t index(int row,int column)const;
};
}
