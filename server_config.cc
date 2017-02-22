#include "server_config.h"
#include "http_constants.h"

ServerConfig::~ServerConfig() {
  if (config_) {
    delete config_; 
  }
}

bool ServerConfig::parse_config(const char* arg)
{
  config_ = new NginxConfig; 
  NginxConfigParser parser;

  if (!parser.Parse(arg, config_)) {
    std::cerr << "Unable to parse config file. Check formatting.\n"; 
    return false; 
  }
  return true;
}

int ServerConfig::get_port() {
  std::string token;
  std::string value; 
  for (int i = 0; i < config_->statements_.size(); i++) {
    std::vector<std::string> token_list = config_->statements_[i]->tokens_; 
    if (token_list.size() < 2) {
      continue; 
    }
    token = token_list[0]; 
    value = token_list[1];
    if (token == PORT) {
      if (isdigit(value[0])) {
        port_ = std::stoi(value); 
      }
    }
  }
  return port_; 
}

bool ServerConfig::well_formed_uri(std::string uri) {
  int size = uri.size();
  if (uri[0] != '/' || uri[size-1] == '/') {
    return false; 
  }
  // Check for repeated '/' in a row
  for (int i = 0; i < size; i++) {
    if (uri[i] == '/' && i != (size-1)) {
      if (uri[i+1] == '/') {
        return false; 
      }
    }
  }
  return true;
}

bool ServerConfig::get_handler_config(std::shared_ptr<NginxConfigStatement>& statement, std::unique_ptr<NginxConfig>& handler_config) {
  if (statement->child_block_ == nullptr) {
    return false; 
  }
  handler_config = std::move(statement->child_block_); 
  return true;
}

std::string ServerConfig::handler_map_content() {
  std::string content = "";
  for (const auto &p : handler_map_) {
   content += "handler_map_[" + p.first + "] = " + p.second->uri() + "\n";
  } 
  return content;
}

bool ServerConfig::build_handlers() {
  for (int i = 0; i < config_->statements_.size(); i++) {
    std::vector<std::string> token_list = config_->statements_[i]->tokens_;
    if (token_list.size() < 3) {
      continue; 
    }

    std::string token = token_list[0];
    std::string uri = token_list[1]; 
    std::string handler = token_list[2];

    if (token == PATH) {

      // Manage URI 
      if(!well_formed_uri(uri)) {
        BOOST_LOG_TRIVIAL(warning) << uri << " is not well formed. Ignoring path block.";
        continue;  
      }

      auto it = handler_map_.find(uri); 
      // TODO: Do longest matching prefix check here
      // If /static1/foo exists already and we are attempting to add
      // /static1 to the map, we will state that /static1 exists already
      // because /static1/foo is the longest matching prefix
      
      if (it != handler_map_.end()) {
        BOOST_LOG_TRIVIAL(warning) << uri << " exists already. Ignoring path block";
        continue;
      }

      // Pass in config block to handler
      std::unique_ptr<NginxConfig> handler_config; 
      if (!get_handler_config(config_->statements_[i], handler_config)) {
        BOOST_LOG_TRIVIAL(warning) << handler << " is missing child config block {...}. Ignoring path block.";
        continue;
      }

      auto req_handler = RequestHandler::CreateByName(handler.c_str()); 

      // TODO: Incorporate the 'default NotFoundHandler{}'
      if (req_handler == nullptr) {
        BOOST_LOG_TRIVIAL(warning) << handler << " is not implemented. Ignoring path block.";
        continue; 
      }

      RequestHandler::Status init = req_handler->Init(uri, *handler_config);
      
      if (init == RequestHandler::OK) {
        BOOST_LOG_TRIVIAL(info) << "Init function successfully called"; 
      }
      else if (init == RequestHandler::MISSING_ROOT) {
        BOOST_LOG_TRIVIAL(warning) << "StaticHandler missing root path. Ignoring path block";
        continue; 
      }
      else if (init == RequestHandler::INVALID_PATH) {
        BOOST_LOG_TRIVIAL(warning) << "StaticHandler path is invalid. Ignoring path block";
        continue;
      }

      std::shared_ptr<RequestHandler> tmp(req_handler);
      handler_map_[uri] = tmp;

    }
  }

  if (handler_map_.size() == 0) {
    return false; 
  }
  
  BOOST_LOG_TRIVIAL(info) << "Handler map content: \n" << handler_map_content();
  return true;
}

std::shared_ptr<RequestHandler> ServerConfig::get_handler(std::string uri) {
  auto it = handler_map_.find(uri);
  if (it != handler_map_.end()) {
     return it->second;
  }
  return nullptr;
}

