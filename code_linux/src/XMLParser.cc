#include <string.h>
//#include <libxml++.h>
//#include <libxml++/parsers/domparser.h>
///#include <sbml/SBMLTypes.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <ctype.h>
#include <locale>
#include <set>
#include <unordered_map>
#include "../include/XMLParser.h"
#include "../include/GraphManager.h"
#include "../include/GraphManagerNew.h"

using namespace std;
///LIBSBML_CPP_NAMESPACE_USE;

//for being able to print map
template<typename Key, typename Val>
extern void print_map(map<Key, Val>& map2print);
extern set<string> split_string(string name_str, string delim);
extern void split_string_into_list(string name_str, string delim, list<string>& list_of_strings);
extern std::string concatenate_strings(std::vector<std::string> vec_strings, std::string delim);
extern string remove_substr(string base_string, string substr_to_remove);

//this comparator is needed so that the complex id names etc follow a consistent order of the component names
struct ComparatorForComplex {
	bool operator() (Node* x ,Node* y) { return (x->rep_id_name < y->rep_id_name);}
} compObject;


XMLParser::XMLParser() : xmlpp::DomParser() {
}

// comment out when using SBML parser
XMLParser::~XMLParser() {
}

void XMLParser::cartestian_product(vector<set<int> >& components_set, set<int>& comp_member_node_set){
	if(components_set.empty()){
		for(set<int>::iterator set_itr = comp_member_node_set.begin(); set_itr != comp_member_node_set.end(); set_itr++){
			set<int> temp_set;
			temp_set.insert(*set_itr);
			components_set.push_back(temp_set);
		}
	}
	else{
		vector<set<int> >temp(components_set);
		components_set.clear();

		for(set<int>::iterator set_itr = comp_member_node_set.begin(); set_itr != comp_member_node_set.end(); set_itr++){
			for(vector<set<int> >::iterator vec_set_itr = temp.begin(); vec_set_itr != temp.end(); vec_set_itr++){
				set<int> temp_set(*vec_set_itr);
				temp_set.insert(*set_itr);
				components_set.push_back(temp_set);
			}
		}
	}
}

//void SBMLParser::cartestian_product_sbml(vector<set<int> >& components_set, set<int>& comp_member_node_set){
//	if(components_set.empty()){
//		for(set<int>::iterator set_itr = comp_member_node_set.begin(); set_itr != comp_member_node_set.end(); set_itr++){
//			set<int> temp_set;
//			temp_set.insert(*set_itr);
//			components_set.push_back(temp_set);
//		}
//	}
//	else{
//		vector<set<int> >temp(components_set);
//		components_set.clear();
//
//		for(set<int>::iterator set_itr = comp_member_node_set.begin(); set_itr != comp_member_node_set.end(); set_itr++){
//			for(vector<set<int> >::iterator vec_set_itr = temp.begin(); vec_set_itr != temp.end(); vec_set_itr++){
//				set<int> temp_set(*vec_set_itr);
//				temp_set.insert(*set_itr);
//				components_set.push_back(temp_set);
//			}
//		}
//	}
//}
vector<int> XMLParser::get_component_list_for_xml_node(xmlpp::Node* node){
	vector<int> component_list;
	const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
	string type = nodeElement->get_attribute_value("type");
	if (type == "group") {
			// Get ids of components of node
			xmlpp::Node::NodeList node_comp_list = nodeElement->get_children("component");
			int num_of_components = 0;
			for(xmlpp::Node::NodeList::iterator comp_iter = node_comp_list.begin(); comp_iter != node_comp_list.end(); comp_iter++) {
				const xmlpp::Element* compElement = dynamic_cast<const xmlpp::Element*>(*comp_iter);
				int comp_id = atoi(compElement->get_attribute_value("id").c_str());
				component_list.push_back(comp_id);
				num_of_components++;
			}
			assert(num_of_components >= 1);
		}
	return component_list;
}

bool XMLParser::cycle_in_components_for_node(int node_id, std::map<int, xmlpp::Node*>& file_id_node_map, set<int>& visited_nodes){
	set<int>::iterator itr = visited_nodes.find(node_id);
	if(itr != visited_nodes.end()){//cycle detected
		return true;
	}
	else{
		visited_nodes.insert(node_id);
		std::map<int, xmlpp::Node*>::iterator map_itr = file_id_node_map.find(node_id);
		if(map_itr == file_id_node_map.end()){//node not even in the file -- why this case - bug in file???
			cerr << "the id " << node_id << " is not present in the file, exiting ... " << endl;
			exit(1);
		}
		vector<int> components_list = get_component_list_for_xml_node(map_itr->second);
		for(vector<int>::iterator cl_itr = components_list.begin(); cl_itr != components_list.end(); cl_itr++){
			bool is_cyle =  cycle_in_components_for_node(*cl_itr, file_id_node_map, visited_nodes);
			if(is_cyle){
				return true;
			}
		}
	}
	return false;
}


//brute force - can be optimised later
bool XMLParser::cycle_in_components_relations(std::map<int, xmlpp::Node*>& file_id_node_map){
	for(std::map<int, xmlpp::Node*>::iterator node_map_itr = file_id_node_map.begin(); node_map_itr != file_id_node_map.end(); node_map_itr++){
		set<int> visited_nodes;
		bool is_cyle = cycle_in_components_for_node(node_map_itr->first, file_id_node_map, visited_nodes);
		if(is_cyle){
			return true;
		}
	}
	return false;
}

//////void SBMLParser::create_map_from_sbml_file(ofstream& mymap, string sbmlfile){
//////        SBMLReader* reader = new SBMLReader();
////// 	SBMLDocument *document = reader->readSBMLFromFile(sbmlfile.c_str());
////// 	unsigned int level = document->getLevel ();
////// 	unsigned int version = document->getVersion();
////// 	cout << endl
////// 	<< "File: " << sbmlfile
////// 	<< " (Level " << level << ", version " << version << ")" << endl;
////// 	Model* model = document->getModel();
//////        string pathway = model->getId();
//////        
////// 	if (model == 0) {
////// 		cout << "No model present." << endl;
////// 	}
////// 	cout << ""
////// 	<< (level == 1 ? "name: " : " id: ")
////// 	<< (model->isSetId() ? model->getId() : "(empty)") << endl;
////// 			
////// 	ListOfSpecies* list_of_species = model->getListOfSpecies();
//////        //assert(list_of_species->size()!=0);
//////        
//////        //create a map of file species id (as in SBML file) to node for quick access to source and target nodes while iterating over the reactions
//////        unordered_map<string, int> species_id_to_nid_map;
//////        for(int index = 0; index < list_of_species->size(); index++){
//////                const Species * species = list_of_species->get(index);
//////                XMLNode * annotation = species->getAnnotation();
//////                XMLNode IS = annotation->getChild("RDF").getChild("Description").getChild("is").getChild("Bag");
//////
//////                string sname = species->getName();
//////                string repid = species->getId();
//////                repid.erase (repid.begin(), repid.begin()+8); 
//////                repid = "R-HSA-" + repid;
//////        
//////                for(int is_index = 0; is_index < IS.getNumChildren(); is_index++){
//////                    XMLNode is = IS.getChild(is_index);
//////                    string sub1 = remove_substr(is.getAttrValue(0),"urn:miriam:");
//////                    if(sub1.substr(0,8) == "uniprot:")
//////                        mymap << repid << "\t" << sub1.substr(8) << endl;
//////
//////                }
//////
//////        }
//////    
//////}

//////bool SBMLParser::fill_graph_from_sbml_file(GraphNew* graph_ptr, string inputfilename, GraphManagerNew* graph_man) {
//////    
//////              
//////        SBMLReader* reader = new SBMLReader();
////// 	SBMLDocument *document = reader->readSBMLFromFile(inputfilename.c_str());
//////        
////// 	unsigned int level = document->getLevel ();
////// 	unsigned int version = document->getVersion();
////// 	//cout << endl
////// 	//<< "File: " << inputfilename
////// 	//<< " (Level " << level << ", version " << version << ")" << endl;
////// 	Model* model = document->getModel();
//////        string pathway = model->getId();
//////        //cout << "pathway is " << pathway << endl;
////// 	if (model == 0) {
////// 		cout << "No model present." << endl;
////// 	}
//////// 	cout << ""
//////// 	<< (level == 1 ? "name: " : " id: ")
//////// 	<< (model->isSetId() ? model->getId() : "(empty)") << endl;
////// 			
////// 	ListOfSpecies* list_of_species = model->getListOfSpecies();
//////        //assert(list_of_species->size()!=0);
//////        
//////        //create a map of file species id (as in SBML file) to node for quick access to source and target nodes while iterating over the reactions
//////        unordered_map<string, int> species_id_to_nid_map;
//////        int species_size = list_of_species->size();
//////        for(int index = 0; index < species_size; index++){
//////            const Species * species = list_of_species->get(index);
//////            
//////            int nid = process_node_sbml(graph_ptr, species, graph_man);
//////            species_id_to_nid_map[species->getId()] = nid;
//////            
//////        }
//////        //assert(!file_id_species_map.empty());
//////                        
////////        if(cycle_in_components_relations(file_id_species_map)){
////////		cerr << "Error: cycle detected in components relation in the graph: " + input_file << endl;
////////		graph_man->destroy_graph(graph_ptr);
////////		return false;
////////	}
//////        
//////        //edges
//////        ListOfReactions* list_of_reactions = model->getListOfReactions();
//////        //assert(list_of_reactions->size()!=0);
//////        int reaction_size = list_of_reactions->size();
//////        for(int index = 0; index < reaction_size; index++){
//////            const Reaction * reaction = list_of_reactions->get(index);
//////            string reaction_id = remove_substr(reaction->getId(), "reaction_");
//////            //int reac_id  = stringToInteger(reaction_id);
//////            //cout << reaction->getId() << " " << reac_id;
//////            int reac_node_id = process_node_sbml(graph_ptr, reaction, graph_man);
//////            
//////            //int reac_node_nid = *reaction_node_id_set.begin();
//////            std::set<string> modifiers_node_ids_set, reactants_node_ids_set, products_node_ids_set;
//////            
//////            const ListOfSpeciesReferences * list_of_modifiers = reaction->getListOfModifiers(); // modifiers are considered source for an edge
//////            int modifiers_size = list_of_modifiers->size();
//////            for (int i = 0; i < modifiers_size; i++){
//////
//////                modifiers_node_ids_set.insert(list_of_modifiers->get(i)->getSpecies());
//////                
//////            }
//////
//////            const ListOfSpeciesReferences * list_of_reactants = reaction->getListOfReactants(); // reactants are considered source for an edge
//////            int reactants_size = list_of_reactants->size();
//////            for (int i = 0; i < reactants_size; i++){
//////
//////                reactants_node_ids_set.insert(list_of_reactants->get(i)->getSpecies());
//////            }
//////            
//////            const ListOfSpeciesReferences * list_of_products = reaction->getListOfProducts(); // products are considered target for an edge
//////            int products_size = list_of_products->size();
//////            for (int i = 0; i < products_size; i++){
//////
//////                products_node_ids_set.insert(list_of_products->get(i)->getSpecies());
//////                
//////            }
//////            //cout <<source_node_ids_set.size() << " - " << target_node_ids_set.size() << endl;
//////
//////            //string source_display_name = species->getName();
//////
//////            for(std::set<string>::iterator reac_itr = reactants_node_ids_set.begin(); reac_itr != reactants_node_ids_set.end(); reac_itr++){
//////                //int source_nid = graph_ptr->get_nid_from_rep_id(*reac_itr);
//////                int source_nid = species_id_to_nid_map[*reac_itr];
//////                if (source_nid != 0){
//////                    process_edge_sbml(graph_ptr, source_nid, reac_node_id, reaction, graph_man, pathway, 0);			
//////                }
//////                    
//////            }
//////            
//////            for(std::set<string>::iterator mod_itr = modifiers_node_ids_set.begin(); mod_itr != modifiers_node_ids_set.end(); mod_itr++){
//////                int source_nid = species_id_to_nid_map[*mod_itr];
//////                if (source_nid != 0){
//////                    process_edge_sbml(graph_ptr, source_nid, reac_node_id, reaction, graph_man, pathway, 0);			
//////                }
//////            }
//////            
//////            for(std::set<string>::iterator prod_itr = products_node_ids_set.begin(); prod_itr != products_node_ids_set.end(); prod_itr++){
//////                int target_nid = species_id_to_nid_map[*prod_itr];
//////                if (target_nid != 0){
//////                    process_edge_sbml(graph_ptr, reac_node_id, target_nid, reaction, graph_man, pathway, 0);			
//////                }
//////            }
//////            
//////            
//////
//////        }
//////                        
//////        return true;
////// 
//////}
////////vector<int> SBMLParser::get_component_list_for_sbml_node(Species* species){
////////	vector<int> component_list;
////////	const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
////////	string type = nodeElement->get_attribute_value("type");
////////	if (type == "group") {
////////			// Get ids of components of node
////////			xmlpp::Node::NodeList node_comp_list = nodeElement->get_children("component");
////////			int num_of_components = 0;
////////			for(xmlpp::Node::NodeList::iterator comp_iter = node_comp_list.begin(); comp_iter != node_comp_list.end(); comp_iter++) {
////////				const xmlpp::Element* compElement = dynamic_cast<const xmlpp::Element*>(*comp_iter);
////////				int comp_id = atoi(compElement->get_attribute_value("id").c_str());
////////				component_list.push_back(comp_id);
////////				num_of_components++;
////////			}
////////			assert(num_of_components >= 1);
////////		}
////////	return component_list;
////////}
//////
//////
//////// create a species node from sbml
//////int SBMLParser::process_node_sbml(GraphNew* graph_ptr, const Species * species, GraphManagerNew* graph_man){
//////        
//////    int nid;
//////    list<string> all_ids;
//////    
//////    // Erase invalid dot characters from id name
//////    //char chars[] = ":";
//////    XMLNode * annotation = species->getAnnotation();
//////    XMLNode IS = annotation->getChild("RDF").getChild("Description").getChild("is").getChild("Bag");
//////
//////    string sname = species->getName();
//////    string repid = species->getId();
//////    repid.erase (repid.begin(), repid.begin()+8); 
//////    repid = "R-HSA-" + repid;
//////
//////    //all_ids.push_back(repid);
//////    
//////    //set<string> all_display_names;
//////    string display_name = species->getName();
//////    //all_display_names.insert(display_name);
//////    
//////    vector<string> components_list;//contains only the xml file ids of the component nodes for now
//////    //components_list = get_component_list_for_sbml_node(species);
//////     
//////    int num_of_components = 0;
//////    XMLNode HAS_PART = annotation->getChild("RDF").getChild("Description").getChild("hasPart").getChild("Bag");
//////    for(int has_index = 0; has_index < HAS_PART.getNumChildren(); has_index++){
//////        XMLNode has = HAS_PART.getChild(has_index);
//////        
//////        components_list.push_back(remove_substr(has.getAttrValue(0),"urn:miriam:"));
//////        num_of_components++;
//////    }
//////    assert(HAS_PART.getNumChildren() == num_of_components);
//////    
//////    
//////    //for(list<string>::iterator all_id_itr = all_ids.begin(); all_id_itr != all_ids.end(); all_id_itr++){
//////        //graph_man->add_rep_id_for_id(*all_id_itr, repid);
//////    //}
//////    graph_man->add_rep_id_for_id(repid, repid);
//////    nid = graph_ptr->get_nid_from_rep_id(repid);
//////    if(nid == -1){
//////            nid = graph_ptr->create_new_node();
//////            graph_ptr->add_node_id(nid);
//////            graph_ptr->add_rep_id_for_node(nid, repid);
//////            graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
//////            //vector<string> all_id_for_rid = graph_man->get_all_ids_for_rep_id(repid);
//////            //for(vector<string>::iterator all_id_for_rid_itr = all_id_for_rid.begin(); all_id_for_rid_itr != all_id_for_rid.end(); all_id_for_rid_itr++){
//////                    //graph_ptr->add_id_for_node(nid, *all_id_for_rid_itr);
//////                    //graph_ptr->add_id_rep_id(*all_id_for_rid_itr, repid);
//////            //}
//////            graph_ptr->add_id_for_node(nid, repid);
//////            graph_ptr->add_id_rep_id(repid, repid);
//////            string type = "protein";
//////            graph_ptr->add_node_type(nid, type);
//////            //for(set<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
//////                    graph_ptr->add_display_id_for_node(nid, display_name);
//////            //}
//////
//////    }
//////			
//////    return nid;
//////}
//////
///////*
//////int SBMLParser::process_node_sbml(GraphNew* graph_ptr, const Species * species, GraphManagerNew* graph_man){
//////        
//////    //std::set<int> nodes_set;
//////    int nid;
//////    list<string> all_ids;
//////    // Erase invalid dot characters from id name
//////    char chars[] = ":";
//////    XMLNode * annotation = species->getAnnotation();
//////    XMLNode IS = annotation->getChild("RDF").getChild("Description").getChild("is").getChild("Bag");
//////
////////    for(int is_index = 0; is_index < IS.getNumChildren(); is_index++){
////////        XMLNode is = IS.getChild(is_index);
////////        all_ids.push_back(remove_substr(is.getAttrValue(0),"urn:miriam:"));
////////        
////////    }
////////    cout << "all_ids: " << all_ids.size() << endl;
//////    string sname = species->getName();
//////    set<string> all_display_names;
//////       
//////    string display_name = species->getName();
//////    std::string::size_type bracket_pos = display_name.find('[');
//////    string name_symbol = display_name.substr(0, bracket_pos);
//////    
////////    for (unsigned int i = 0; i < strlen(chars); ++i) {
////////		(sname).erase (std::remove((sname).begin(), (sname).end(), chars[i]), (sname).end());
////////	}
//////    all_display_names.insert(display_name);
//////    
//////
//////    string name_symbol_short = name_symbol.substr(0, name_symbol.length()-1);
//////    cout << "#" << name_symbol_short << "#" << endl;
//////    
//////
//////    all_ids.push_back(name_symbol_short);
//////    
//////    // For nodes that are complexes fill components
//////	vector<string> components_list;//contains only the xml file ids of the component nodes for now
//////	//components_list = get_component_list_for_sbml_node(species);
//////        
//////     
//////    int num_of_components = 0;
//////    XMLNode HAS_PART = annotation->getChild("RDF").getChild("Description").getChild("hasPart").getChild("Bag");
//////    for(int has_index = 0; has_index < HAS_PART.getNumChildren(); has_index++){
//////        XMLNode has = HAS_PART.getChild(has_index);
//////        
//////        components_list.push_back(remove_substr(has.getAttrValue(0),"urn:miriam:"));
//////        num_of_components++;
//////    }
//////    assert(HAS_PART.getNumChildren() == num_of_components);
//////                            
//////    
//////    //if the nodes is not a complex(group)
//////	///if(components_list.empty()){
//////            
////////		set<string> rep_ids_set;//set of rep ids that are there in this node
////////                string repid = all_ids.front();
////////                //string repid = species->getId();
////////                string repid_from_map = graph_man->get_rep_id_for_id(repid);
////////                if(repid_from_map != "")
////////                    repid = repid_from_map;
////////                
////////                cout << "REP: " << repid << endl;
////////                rep_ids_set.insert(repid);
////////                
//////                
//////                set<string> rep_ids_set;//set of rep ids that are there in this node
//////                //string repid = all_ids.front();
//////                string repid = species->getId();
//////                
//////                //
//////                repid.erase (repid.begin(), repid.begin()+8); 
//////                repid = "R-HSA-" + repid;
//////                
//////                             
//////                string reac_id = "reactome:" + repid;
//////                all_ids.push_back(repid);
//////                
//////                
//////                
//////		for(list<string>::iterator all_id_itr = all_ids.begin(); all_id_itr != all_ids.end(); all_id_itr++){
////////                    
////////			//find the current id in the all id map
////////			//map<string, string>::iterator all_id_map_itr = graph_man->all_id_map.find(*all_id_itr);
////////			string repid = graph_man->get_rep_id_for_id(*all_id_itr);
////////			if(repid == ""){
////////				//if the id is not in the all id map then id itself becomes rep id and also inserts this info in all id map
////////				//think of compounds for this case
////////				graph_man->add_rep_id_for_id(*all_id_itr, *all_id_itr);
////////				rep_ids_set.insert(*all_id_itr);
////////				
//////                                graph_man->add_rep_id_for_id(*all_id_itr, repid);
//////                    
////////				
////////			}
////////			else{
////////                                rep_ids_set.insert(repid);
////////			}
//////		}
//////                
//////		for(set<string>::iterator rid_itr = rep_ids_set.begin(); rid_itr != rep_ids_set.end(); rid_itr++){
//////			nid = graph_ptr->get_nid_from_rep_id(*rid_itr);
//////			if(nid == -1){//nid == -1 means repid not there in the graph as of now
//////				//nid = ++GraphManagerNew::node_id_count;
//////                                nid = graph_ptr->create_new_node(); // creates a new node -- added by sukanya
//////                                graph_ptr->add_node_id(nid);
//////                                graph_ptr->add_rep_id_for_node(nid, *rid_itr); // added by sukanya
//////                                graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
//////				vector<string> all_id_for_rid = graph_man->get_all_ids_for_rep_id(*rid_itr);
//////				for(vector<string>::iterator all_id_for_rid_itr = all_id_for_rid.begin(); all_id_for_rid_itr != all_id_for_rid.end(); all_id_for_rid_itr++){
//////					graph_ptr->add_id_for_node(nid, *all_id_for_rid_itr);
//////					graph_ptr->add_id_rep_id(*all_id_for_rid_itr, *rid_itr); // commented out by sukanya
//////				}
//////                                string type = "protein";
//////				graph_ptr->add_node_type(nid, type);
//////				for(set<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
//////					graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
//////				}
//////                                
//////			}
//////			else{
//////				//if the new display names are not there add them
//////				for(set<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
//////					if(!graph_ptr->node_has_display_id(nid, *dis_name_itr)){
//////						graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
//////					}
//////				}
//////			}
//////
//////			//nodes_set.insert(nid);
//////		}
//////	////}
//////	/*else{//if the node is a complex(group)
//////            
//////		vector<set<int> > components_set;
//////		//cycle detection in components already done
//////		for(vector<string>::iterator cl_itr = components_list.begin(); cl_itr != components_list.end(); cl_itr++){
//////                    if(file_id_species_map.find(*cl_itr)!= file_id_species_map.end()){
//////                        set<int> comp_member_node_set = process_node_sbml(graph_ptr, file_id_species_map.find(*cl_itr)->second, graph_man, file_id_species_map);
//////                        cartestian_product_sbml(components_set, comp_member_node_set);
//////                        }
//////                    else {//component node is not yet created
//////                        
//////                        
//////                        int nid = graph_ptr->get_nid_from_rep_id(*cl_itr);
//////                        nid = graph_ptr->create_new_node(); // creates a new node -- added by sukanya
//////                                graph_ptr->add_node_id(nid);
//////                                graph_ptr->add_rep_id_for_node(nid, *cl_itr); // added by sukanya
//////                                graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
//////				vector<string> all_id_for_rid = graph_man->get_all_ids_for_rep_id(*cl_itr);
//////				for(vector<string>::iterator all_id_for_rid_itr = all_id_for_rid.begin(); all_id_for_rid_itr != all_id_for_rid.end(); all_id_for_rid_itr++){
//////					graph_ptr->add_id_for_node(nid, *all_id_for_rid_itr);
//////					graph_ptr->add_id_rep_id(*all_id_for_rid_itr, *cl_itr); // commented out by sukanya
//////				}
//////                                string type = "";
//////				graph_ptr->add_node_type(nid, type);
//////				for(set<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
//////					graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
//////				}
//////                    }
//////		}
//////
//////		for(vector<set<int> >::iterator vec_set_itr = components_set.begin(); vec_set_itr != components_set.end(); vec_set_itr++){
//////                    
//////			int nid = graph_ptr->get_node_id_with_components(*vec_set_itr);
//////			if (nid == -1){
//////				//nid = ++GraphManagerNew::node_id_count;
//////                                nid = graph_ptr->create_new_node(); // creates a new node -- added by sukanya
//////				graph_ptr->add_node_id(nid);
//////				graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
//////				string complex_id = graph_ptr->concatenate_sorted_rep_ids(*vec_set_itr, "_");
//////				complex_id = "c_s_" + complex_id + "_c_e";
//////				graph_man->add_rep_id_for_id(complex_id, complex_id);
//////				graph_ptr->add_id_for_node(nid, complex_id);
//////                                graph_ptr->add_rep_id_for_node(nid, complex_id); // added by sukanya
//////				graph_ptr->add_id_rep_id(complex_id, complex_id); // commented out by sukanya
//////                                string type = "";
//////				graph_ptr->add_node_type(nid, type);
//////				for(set<int>::iterator comp_mem_itr = vec_set_itr->begin(); comp_mem_itr != vec_set_itr->end(); comp_mem_itr++){
//////					graph_ptr->add_component_id_for_node(nid, *comp_mem_itr);
//////				}
//////				graph_ptr->add_display_id_for_node(nid, complex_id);
//////			}
//////			nodes_set.insert(nid);
//////		}
//////	}
//////    //return nodes_set;
//////    return nid;
//////}
//////*/
//////// create a reaction node from sbml
//////int SBMLParser::process_node_sbml(GraphNew* graph_ptr, const Reaction * species, GraphManagerNew* graph_man){
//////        
//////    int nid;
//////    list<string> all_ids;
//////    // Erase invalid dot characters from id name
//////    char chars[] = ":";
//////    XMLNode * annotation = species->getAnnotation();
//////    XMLNode IS = annotation->getChild("RDF").getChild("Description").getChild("is").getChild("Bag");
//////
//////    
//////    for(int is_index = 0; is_index < IS.getNumChildren(); is_index++){
//////        XMLNode is = IS.getChild(is_index);
//////        all_ids.push_back(remove_substr(is.getAttrValue(0),"urn:miriam:"));
//////        
//////    }
//////    
//////    string repid = species->getId();
//////    repid.erase (repid.begin(), repid.begin()+9);
//////    repid = "R-HSA-" + repid;
//////    
//////    
//////    graph_man->add_rep_id_for_id(repid, repid);
//////
//////    nid = graph_ptr->get_nid_from_rep_id(repid);
//////    if(nid == -1){
//////            nid = graph_ptr->create_new_node();
//////            graph_ptr->add_node_id(nid);
//////            graph_ptr->add_rep_id_for_node(nid, repid);
//////            graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
////////            vector<string> all_id_for_rid = graph_man->get_all_ids_for_rep_id(repid);
////////            for(vector<string>::iterator all_id_for_rid_itr = all_id_for_rid.begin(); all_id_for_rid_itr != all_id_for_rid.end(); all_id_for_rid_itr++){
////////                    graph_ptr->add_id_for_node(nid, *all_id_for_rid_itr);
////////                    graph_ptr->add_id_rep_id(*all_id_for_rid_itr, repid);
////////            }
//////            graph_ptr->add_id_for_node(nid, repid);
//////            graph_ptr->add_id_rep_id(repid, repid);
//////            string type = "reaction";
//////            string compartment = "";
//////            graph_ptr->add_node_type(nid, type);
//////            graph_ptr->add_display_id_for_node(nid, repid);
//////            graph_ptr->add_description_for_node(nid, species->getName());
//////            graph_ptr->add_compartment_for_node(nid, compartment);
//////    }
//////			
//////
//////	
//////    return nid;
//////}

bool XMLParser::fill_graph_from_xml_file(GraphNew* graph_ptr, string input_file, GraphManagerNew* graph_man, int thres, vector<string>& MAPK_merged_ids, map<string, set<string> >& MAPK_node_to_pathway_id_map, bool tool_written){
    
            
    
	//parse the xml flie
	xmlpp::DomParser parser;
	parser.parse_file(input_file);
	const xmlpp::Node* pNode = parser.get_document()->get_root_node();
	const xmlpp::Element* rootElement = dynamic_cast<const xmlpp::Element*>(pNode);
	////string pathway = rootElement->get_attribute_value("name");
        string pathway = rootElement->get_attribute_value("name");
        
	//get all the nodes from the xml file
	// For nodes in XML file
	xmlpp::Node::NodeList nodes_list = pNode->get_children("entry");
#ifdef ASSERT_FLAG
	assert(!nodes_list.empty());
#endif
        //cout << "Read " << nodes_list.size() << " nodes from xml file" << endl;

	//create a map of file node id (as in XML file ) to node for quick access to source and target nodes while iterating over the edges
	std::map<int, xmlpp::Node*> file_id_node_map;
	for(xmlpp::Node::NodeList::iterator iter = nodes_list.begin(); iter != nodes_list.end(); ++iter) {
		const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(*iter);
		int id = atoi(nodeElement->get_attribute_value("id").c_str());
		file_id_node_map[id] = *iter;
                
                
                
	}
#ifdef ASSERT_FLAG
	assert(!file_id_node_map.empty());
#endif


        // temporarily commencted out by sukanya on 4 Aug 2016
//	if(cycle_in_components_relations(file_id_node_map)){
//		cerr << "Error: cycle detected in components relation in the graph: " + input_file << endl;
//		graph_man->destroy_graph(graph_ptr);
//		return false;
//	}

        
	//now iterate over each edge and fill graph
        int count = 0;
	xmlpp::Node::NodeList edges_list = pNode->get_children("relation");
	for(xmlpp::Node::NodeList::iterator iter = edges_list.begin(); iter != edges_list.end(); ++iter) {
                
		const xmlpp::Element* edgeElement = dynamic_cast<const xmlpp::Element*>(*iter);
		const xmlpp::Element::AttributeList& edge_attr_list = edgeElement->get_attributes();
		xmlpp::Node::NodeList edge_subtype_list = edgeElement->get_children("subtype");

		int file_id_source = atoi(edgeElement->get_attribute_value("entry1").c_str());
		int file_id_target = atoi(edgeElement->get_attribute_value("entry2").c_str());
		xmlpp::Node* source_node = file_id_node_map[file_id_source];
		xmlpp::Node* target_node = file_id_node_map[file_id_target];

//              std::set<int> source_node_ids_set = process_node(graph_ptr, source_node, graph_man, file_id_node_map);
//		std::set<int> target_node_ids_set = process_node(graph_ptr, target_node, graph_man, file_id_node_map);
		//std::set<int> source_node_ids_set = process_node(graph_ptr, source_node, graph_man, file_id_node_map, thres);
		//std::set<int> target_node_ids_set = process_node(graph_ptr, target_node, graph_man, file_id_node_map, thres);
                std::set<int> source_node_ids_set = process_node_with_merge_exception(graph_ptr, source_node, graph_man, file_id_node_map, thres, pathway, MAPK_merged_ids, MAPK_node_to_pathway_id_map);
		std::set<int> target_node_ids_set = process_node_with_merge_exception(graph_ptr, target_node, graph_man, file_id_node_map, thres, pathway, MAPK_merged_ids, MAPK_node_to_pathway_id_map);
                
                assert(source_node_ids_set.size() != 0);
                assert(target_node_ids_set.size() != 0);
                
                
//                if(tool_written) {
//                   pathway = edgeElement->get_attribute_value("pathways");
//                   
//                }
//            
                
                //cout << source_node_ids_set.size() << " " << target_node_ids_set.size() << endl;
//////#ifdef ASSERT_FLAG
//////                assert(!tool_written || ((source_node_ids_set.size() == 1) && (target_node_ids_set.size() == 1)));
//////#endif
                int num_edges; string check;
		for(std::set<int>::iterator s_itr = source_node_ids_set.begin(); s_itr != source_node_ids_set.end(); s_itr++){
                        for(std::set<int>::iterator t_itr = target_node_ids_set.begin(); t_itr != target_node_ids_set.end(); t_itr++){
				process_edge(graph_ptr, *s_itr, *t_itr, *iter, graph_man, pathway, tool_written);
                                
                                count++;
                                num_edges = graph_ptr->get_edge_ids().size();
//                                if(count != num_edges){
//                                    cout << "Count is " << count << " and " << num_edges << " number of edges in the graph" << endl;
//                                    cin >> check;
//                                }
                                //cout << "Read " << count << " edges from xml file" << endl;    
			}
                       
		}
	}
        //cout << "Read " << count << " edges from xml file" << endl; 
        if (count == 0) {
            cout << input_file << " has no edges" << endl;
        }

        
	return true;
}


//std::set<int> XMLParser::process_node(GraphNew* graph_ptr, xmlpp::Node* node, GraphManagerNew* graph_man, std::map<int, xmlpp::Node*>& file_id_node_map){
//	std::set<int> nodes_set;
//	const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
//
//	//get all the labels for the node
//	string name = nodeElement->get_attribute_value("name"); // hsa ids in xml file
//	// Erase invalid dot characters from id name
//	char chars[] = ":";
//	for (unsigned int i = 0; i < strlen(chars); ++i) {
//		name.erase (std::remove(name.begin(), name.end(), chars[i]), name.end());
//	}
//
//	set<string> all_ids = split_string(name, " ");
//
//        // -- added by sukanya on 14 Jul 2016
////        if (all_ids.size() > 50) {
////            cout << *all_ids.begin() << endl;
////            char wait;
////            cin >> wait;
////        }
//        // --
//	string type = nodeElement->get_attribute_value("type");
//                
//	string display_names;
//	// get display names
//        //postpone reading display names till nodes are split on hsa-ids - modification introduced by sukanya - 1 Oct 2015
////	xmlpp::Node::NodeList node_subtype_list1 = nodeElement->get_children("graphics");
////	for(xmlpp::Node::NodeList::iterator node_subtype_iter1 = node_subtype_list1.begin(); node_subtype_iter1 != node_subtype_list1.end(); ++node_subtype_iter1) {
////		const xmlpp::Element* nodeSubtypeElement1 = dynamic_cast<const xmlpp::Element*>(*node_subtype_iter1);
////		display_names = nodeSubtypeElement1->get_attribute_value("name");
////	}
////	set<string> all_display_names = split_string(display_names, ", ");
////        
//        
//        
//	// For nodes that are complexes fill components
//	vector<int> components_list;//contains only the xml file ids of the component nodes for now
//	components_list = get_component_list_for_xml_node(node);
//
//
//	//if the nodes is not a complex(group)
//	if(components_list.empty()){
//		set<string> rep_ids_set;//set of rep ids that are there in this node
//		for(set<string>::iterator all_id_itr = all_ids.begin(); all_id_itr != all_ids.end(); all_id_itr++){
//			//find the current id in the all id map
//			map<string, string>::iterator all_id_map_itr = (graph_man->all_id_map).find(*all_id_itr);
//			string repid = graph_man->get_rep_id_for_id(*all_id_itr);
//                        
//			if(repid == ""){
//				//if the id is not in the all id map then id itself becomes rep id and also inserts this info in all id map
//				//think of compounds for this case
//				//(graph_man->all_id_map)[*all_id_itr] = *all_id_itr;
//				graph_man->add_rep_id_for_id(*all_id_itr, *all_id_itr);
//				rep_ids_set.insert(*all_id_itr);
//                            
//                            
//			}
//			else{
//				rep_ids_set.insert(repid);
//			}
//		}
//
//		for(set<string>::iterator rid_itr = rep_ids_set.begin(); rid_itr != rep_ids_set.end(); rid_itr++){
//			int nid = graph_ptr->get_nid_from_rep_id(*rid_itr);
//                        
//                        if(nid == -1){//nid == -1 means repid not there in the graph as of now
//				//nid = ++GraphManagerNew::node_id_count;
//                                nid = graph_ptr->create_new_node(); // creates a new node -- added by sukanya
//                                graph_ptr->add_node_id(nid);
//                                graph_ptr->add_rep_id_for_node(nid, *rid_itr); // added by sukanya
//                                graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
//				vector<string> all_id_for_rid = graph_man->get_all_ids_for_rep_id(*rid_itr);
//				for(vector<string>::iterator all_id_for_rid_itr = all_id_for_rid.begin(); all_id_for_rid_itr != all_id_for_rid.end(); all_id_for_rid_itr++){
//					graph_ptr->add_id_for_node(nid, *all_id_for_rid_itr);
//					graph_ptr->add_id_rep_id(*all_id_for_rid_itr, *rid_itr); // commented out by sukanya
//				}
//				graph_ptr->add_node_type(nid, type);
//                                // added by sukanya on 1 Oct 2015
//                                
//                                display_names = graph_man->get_display_names_from_rep_id(*rid_itr);
//                                list<string> all_display_names;
//                                split_string_into_list(display_names, ",", all_display_names);
//				for(list<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
//					graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
//				}
//			}
//			else{
//                            	xmlpp::Node::NodeList node_subtype_list1 = nodeElement->get_children("graphics");
//                            	for(xmlpp::Node::NodeList::iterator node_subtype_iter1 = node_subtype_list1.begin(); node_subtype_iter1 != node_subtype_list1.end(); ++node_subtype_iter1) {
//                            		const xmlpp::Element* nodeSubtypeElement1 = dynamic_cast<const xmlpp::Element*>(*node_subtype_iter1);
//                            		display_names = nodeSubtypeElement1->get_attribute_value("name");
//                            	}
//                            	list<string> all_display_names;
//                                split_string_into_list(display_names, ", ", all_display_names);
//				//if the new display names are not there add them
//				for(list<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
//					if(!graph_ptr->node_has_display_id(nid, *dis_name_itr)){
//						graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
//					}
//				}
//			}
//
//			nodes_set.insert(nid);
//		}
//	}
//	else{//if the node is a complex(group)
//		vector<set<int> > components_set;
//		//cycle detection in components already done
//		for(vector<int>::iterator cl_itr = components_list.begin(); cl_itr != components_list.end(); cl_itr++){
//			set<int> comp_member_node_set = process_node(graph_ptr, file_id_node_map.find(*cl_itr)->second, graph_man, file_id_node_map);
//			cartestian_product(components_set, comp_member_node_set);
//		}
//
//		for(vector<set<int> >::iterator vec_set_itr = components_set.begin(); vec_set_itr != components_set.end(); vec_set_itr++){
//			int nid = graph_ptr->get_node_id_with_components(*vec_set_itr);
//			if (nid == -1){
//				//nid = ++GraphManagerNew::node_id_count;
//                                nid = graph_ptr->create_new_node(); // creates a new node -- added by sukanya
//				graph_ptr->add_node_id(nid);
//				graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
//				string complex_id = graph_ptr->concatenate_sorted_rep_ids(*vec_set_itr, "_");
//				complex_id = "c_s_" + complex_id + "_c_e";
//				graph_man->add_rep_id_for_id(complex_id, complex_id);
//				graph_ptr->add_id_for_node(nid, complex_id);
//                                graph_ptr->add_rep_id_for_node(nid, complex_id); // added by sukanya
//				graph_ptr->add_id_rep_id(complex_id, complex_id); // commented out by sukanya
//				graph_ptr->add_node_type(nid, type);
//				for(set<int>::iterator comp_mem_itr = vec_set_itr->begin(); comp_mem_itr != vec_set_itr->end(); comp_mem_itr++){
//					graph_ptr->add_component_id_for_node(nid, *comp_mem_itr);
//				}
//				//graph_ptr->add_display_id_for_node(nid, complex_id);
//                                // -- added by sukanya on 7 Aug 2016
//                                set<int> nid_set_of_complex = *vec_set_itr;
//                                vector<string> comp_disp_names; // used vector to retain the order of nodes
//                                for (auto nid_set_itr = nid_set_of_complex.begin(); nid_set_itr != nid_set_of_complex.end(); nid_set_itr++) {
//                                    comp_disp_names.push_back(graph_ptr->get_all_display_ids_of_node(*nid_set_itr)[0]);
//                                }
//                                string complex_disp_id = concatenate_strings(comp_disp_names, "+");
//                                complex_disp_id = "[" + complex_disp_id + "]";
//                                graph_ptr->add_display_id_for_node(nid, complex_disp_id);
//                                // --
//			}
//			nodes_set.insert(nid);
//		}
//	}
//
//	return nodes_set;
//}


std::set<int> XMLParser::process_node_with_merge_exception(GraphNew* graph_ptr, xmlpp::Node* node, GraphManagerNew* graph_man, std::map<int, xmlpp::Node*>& file_id_node_map, int thres, string pathway, vector<string>& MAPK_merged_ids, map<string, set<string> >& MAPK_node_to_pathway_id_map){
	std::set<int> nodes_set;
	const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
        bool split_node = true;
        
	//get all the labels for the node
	string name = nodeElement->get_attribute_value("name"); // hsa ids in xml file
	// Erase invalid dot characters from id name
	char chars[] = ":";
	for (unsigned int i = 0; i < strlen(chars); ++i) {
		name.erase (std::remove(name.begin(), name.end(), chars[i]), name.end());
	}

	set<string> all_ids = split_string(name, " ");
        
//        // p38 and JNK
//        string p38_id = "hsa1432";
//        string JNK_id = "hsa5599";
//        if ( (all_ids.find(p38_id) != all_ids.end()) && (all_ids.find(JNK_id) != all_ids.end())) {
//            cout << pathway << endl;
//        }
//        
        
        
        if (all_ids.size() > thres)
            split_node = false;
        // --
        // added by sukanya on 2 Aug 2016
        // do not create node if unsplit node already has that id
//        if (!split_node) {
//            for(set<string>::iterator set_itr = all_ids.begin(); set_itr != all_ids.end(); set_itr++) {
//                
//                string rep_id_to_create = graph_ptr->get_rep_id_from_id(*set_itr);
//                if (rep_id_to_create != "") {
//
//                    // following change made on 26 Aug 2016
//                    //nodes_set.clear();
//                    //return nodes_set;
//                    nodes_set.insert(graph_ptr->get_nid_from_rep_id(rep_id_to_create));
//                    return nodes_set;
//                }
//                
//            }
//        }
        
        // --
        
        string all_rep_ids_str = "";
        vector<string> all_ids_vec;
        for(set<string>::iterator set_itr = all_ids.begin(); set_itr != all_ids.end(); set_itr++) {
            if (graph_man->merge_exception_set.find(*set_itr) == graph_man->merge_exception_set.end()) {
                all_ids_vec.push_back(*set_itr);
            
                if (find(MAPK_merged_ids.begin(), MAPK_merged_ids.end(), *set_itr) != MAPK_merged_ids.end()) {
                    MAPK_node_to_pathway_id_map[*set_itr].insert(pathway);
                }
            }
        }
        // create one rep-id if not splitting node
        if (!split_node) {
             ////all_rep_ids_str = concatenate_strings(all_ids_vec, ",");
             //graph_ptr->add_id_rep_id(all_rep_ids_str, all_rep_ids_str);

            if (!all_ids_vec.empty())
                all_rep_ids_str = all_ids_vec[0];
        }
        
	string type = nodeElement->get_attribute_value("type");
                
	string display_names;
	
	// For nodes that are complexes fill components
	vector<int> components_list;//contains only the xml file ids of the component nodes for now
	components_list = get_component_list_for_xml_node(node);


	//if the nodes is not a complex(group)
	if(components_list.empty()){
		set<string> rep_ids_set;//set of rep ids that are there in this node
		for(set<string>::iterator all_id_itr = all_ids.begin(); all_id_itr != all_ids.end(); all_id_itr++){
			//find the current id in the all id map
			//map<string, string>::iterator all_id_map_itr = (graph_man->all_id_map).find(*all_id_itr);
                    
                        if (graph_man->merge_exception_set.find(*all_id_itr) != graph_man->merge_exception_set.end()) {
                            rep_ids_set.insert(*all_id_itr);
                            ////graph_ptr->add_id_rep_id(*all_id_itr, *all_id_itr);
                            continue;
                        }
                        
			string repid = graph_man->get_rep_id_for_id(*all_id_itr);
                        
			if(repid == ""){
                            	//if the id is not in the all id map then id itself becomes rep id and also inserts this info in all id map
				//think of compounds for this case
				//graph_man->add_rep_id_for_id(*all_id_itr, *all_id_itr);
                                // rep_ids_set.insert(*all_id_itr);
                                if (!split_node) {
                                    rep_ids_set.insert(all_rep_ids_str);
                                    //graph_man->add_rep_id_for_id(*all_id_itr, all_rep_ids_str);
                                    graph_ptr->add_id_rep_id(*all_id_itr, all_rep_ids_str);
                                }
                                else {
                                    rep_ids_set.insert(*all_id_itr);
                                    //graph_man->add_rep_id_for_id(*all_id_itr, *all_id_itr);
                                    graph_ptr->add_id_rep_id(*all_id_itr, *all_id_itr);
                                }
                                
                            
			}
			else{
				rep_ids_set.insert(repid);
			}
		}

                
                
		for(set<string>::iterator rid_itr = rep_ids_set.begin(); rid_itr != rep_ids_set.end(); rid_itr++){
			int nid = graph_ptr->get_nid_from_rep_id(*rid_itr);
                        
                        if(nid == -1){//nid == -1 means repid not there in the graph as of now
				//nid = ++GraphManagerNew::node_id_count;
                                nid = graph_ptr->create_new_node();
                                
                                graph_ptr->add_node_id(nid);
                                graph_ptr->add_rep_id_for_node(nid, *rid_itr);
                                if (graph_man->merge_exception_set.find(*rid_itr) != graph_man->merge_exception_set.end()) {
                                    graph_ptr->add_id_rep_id(*rid_itr, *rid_itr);
                                    graph_ptr->add_id_for_node(nid, *rid_itr);
                                    graph_ptr->add_node_type(nid, type);
                                    graph_ptr->add_display_id_for_node(nid, graph_man->kegg_hsa_id_to_display_name_map[*rid_itr]);
                                    graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
                                    nodes_set.insert(nid);
                                    continue;
                                }
                                graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
				//vector<string> all_id_for_rid = graph_man->get_all_ids_for_rep_id(*rid_itr); // changed by sukanya on 25 July 2016
                                ////vector<string> all_id_for_rid = graph_ptr->get_all_ids_for_rep_id(*rid_itr);
                                vector<string> all_id_for_rid = all_ids_vec;
                                for(vector<string>::iterator all_id_for_rid_itr = all_id_for_rid.begin(); all_id_for_rid_itr != all_id_for_rid.end(); all_id_for_rid_itr++){
                                    // assign other ids to node only if those ids are not in exception set    
                                    //if (graph_man->merge_exception_set.find(*all_id_for_rid_itr) == graph_man->merge_exception_set.end()) {
                                    
                                        
					////graph_ptr->add_id_for_node(nid, *all_id_for_rid_itr);  // commented on 27the Sep 2016
					
                                        //graph_ptr->add_id_rep_id(*all_id_for_rid_itr, *rid_itr);
                                        if (!split_node) {
                                            if (all_rep_ids_str == "")
                                                all_rep_ids_str = *all_id_for_rid_itr;
                                            graph_ptr->add_id_rep_id(*all_id_for_rid_itr, all_rep_ids_str);
                                            graph_ptr->add_id_rep_id(all_rep_ids_str, all_rep_ids_str);
                                            graph_ptr->add_id_for_node(nid, *all_id_for_rid_itr); // added on 27the Sep 2016
                                            
                                            if (!graph_ptr->node_has_id(nid, all_rep_ids_str))
                                                graph_ptr->add_id_for_node(nid, all_rep_ids_str);
                                            
                                        }
                                        else {
                                            graph_ptr->add_id_rep_id(*all_id_for_rid_itr, *rid_itr);
                                            if (!graph_ptr->node_has_id(nid, *rid_itr))
                                                graph_ptr->add_id_for_node(nid, *rid_itr); // added on 27the Sep 2016
                                        }
                                        
                                    //}
                                    
//                                    // if id is in exception set, assign only one id to node
//                                    else {
//                                        graph_ptr->add_id_for_node(nid, *rid_itr);
//                                        graph_ptr->add_id_rep_id(*rid_itr, *rid_itr);
//                                    }
				}
//                                graph_ptr->add_id_for_node(nid, all_rep_ids_str);
//                                graph_ptr->add_id_rep_id(all_rep_ids_str, all_rep_ids_str);
                                
				graph_ptr->add_node_type(nid, type);
                                // added by sukanya on 1 Oct 2015
                                
                                display_names = graph_man->get_display_names_from_rep_id(*rid_itr);
                                list<string> all_display_names;
                                split_string_into_list(display_names, ",", all_display_names);
				for(list<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
					graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
				}
			}
			else{
                            	xmlpp::Node::NodeList node_subtype_list1 = nodeElement->get_children("graphics");
                            	for(xmlpp::Node::NodeList::iterator node_subtype_iter1 = node_subtype_list1.begin(); node_subtype_iter1 != node_subtype_list1.end(); ++node_subtype_iter1) {
                            		const xmlpp::Element* nodeSubtypeElement1 = dynamic_cast<const xmlpp::Element*>(*node_subtype_iter1);
                            		display_names = nodeSubtypeElement1->get_attribute_value("name");
                            	}
                            	list<string> all_display_names;
                                split_string_into_list(display_names, ", ", all_display_names);
				//if the new display names are not there add them
				for(list<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
					if(!graph_ptr->node_has_display_id(nid, *dis_name_itr)){
						graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
					}
				}
			}

			nodes_set.insert(nid);
		}
	}
	else{ //if the node is a complex(group)
            
           
		vector<set<int> > components_set;
		//cycle detection in components already done
		for(vector<int>::iterator cl_itr = components_list.begin(); cl_itr != components_list.end(); cl_itr++){
			set<int> comp_member_node_set = process_node_with_merge_exception(graph_ptr, file_id_node_map.find(*cl_itr)->second, graph_man, file_id_node_map, thres, pathway, MAPK_merged_ids, MAPK_node_to_pathway_id_map);
                        cartestian_product(components_set, comp_member_node_set);

		}

                
		for(vector<set<int> >::iterator vec_set_itr = components_set.begin(); vec_set_itr != components_set.end(); vec_set_itr++){
                    
			int nid = graph_ptr->get_node_id_with_components(*vec_set_itr);
			if (nid == -1){
				//nid = ++GraphManagerNew::node_id_count;
                                nid = graph_ptr->create_new_node();
				graph_ptr->add_node_id(nid);
				graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
				string complex_id = graph_ptr->concatenate_sorted_rep_ids(*vec_set_itr, "_");
                                complex_id = "c_s_" + complex_id + "_c_e";
                                
                                // -- added by sukanya on 7 Aug 2016
                                set<int> nid_set_of_complex = *vec_set_itr;
                                vector<string> comp_disp_names; // used vector to retain the order of nodes
                                for (auto nid_set_itr = nid_set_of_complex.begin(); nid_set_itr != nid_set_of_complex.end(); nid_set_itr++) {
                                    comp_disp_names.push_back(graph_ptr->get_all_display_ids_of_node(*nid_set_itr)[0]);
                                }
                                string complex_disp_id = concatenate_strings(comp_disp_names, "+");
                                complex_disp_id = "[" + complex_disp_id + "]";
                                graph_ptr->add_display_id_for_node(nid, complex_disp_id);
                                // --
                                
				graph_man->add_rep_id_for_id(complex_id, complex_id);
				graph_ptr->add_id_for_node(nid, complex_id);
                                graph_ptr->add_rep_id_for_node(nid, complex_id);
				graph_ptr->add_id_rep_id(complex_id, complex_id);
				graph_ptr->add_node_type(nid, type);
				for(set<int>::iterator comp_mem_itr = vec_set_itr->begin(); comp_mem_itr != vec_set_itr->end(); comp_mem_itr++){
					graph_ptr->add_component_id_for_node(nid, *comp_mem_itr);
				}
				//graph_ptr->add_display_id_for_node(nid, complex_id);
                                
			}
			nodes_set.insert(nid);
		}
	}

	return nodes_set;
}

std::set<int> XMLParser::process_node(GraphNew* graph_ptr, xmlpp::Node* node, GraphManagerNew* graph_man, std::map<int, xmlpp::Node*>& file_id_node_map, int thres){
	std::set<int> nodes_set;
	const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
        bool split_node = true;
        
	//get all the labels for the node
	string name = nodeElement->get_attribute_value("name"); // hsa ids in xml file
	// Erase invalid dot characters from id name
	char chars[] = ":";
	for (unsigned int i = 0; i < strlen(chars); ++i) {
		name.erase (std::remove(name.begin(), name.end(), chars[i]), name.end());
	}

	set<string> all_ids = split_string(name, " ");
        
        
        if (all_ids.size() > thres)
            split_node = false;
        // --
        // added by sukanya on 2 Aug 2016
        // do not create node if unsplit node already has that id
        if (!split_node) {
            for(set<string>::iterator set_itr = all_ids.begin(); set_itr != all_ids.end(); set_itr++) {
                string rep_id_to_create = graph_ptr->get_rep_id_from_id(*set_itr);
                if (rep_id_to_create != "") {
                    
                    // following change made on 26 Aug 2016
                    //nodes_set.clear();
                    //return nodes_set;
                    nodes_set.insert(graph_ptr->get_nid_from_rep_id(rep_id_to_create));
                    return nodes_set;
                }
            }
        }
        
        // --
        
        string all_rep_ids_str = "";
        vector<string> all_ids_vec;
        for(set<string>::iterator set_itr = all_ids.begin(); set_itr != all_ids.end(); set_itr++) {
                all_ids_vec.push_back(*set_itr);
        }
        // create one rep-id if not splitting node
        if (!split_node) {
             ////all_rep_ids_str = concatenate_strings(all_ids_vec, ",");
             //graph_ptr->add_id_rep_id(all_rep_ids_str, all_rep_ids_str);

             all_rep_ids_str = all_ids_vec[0];
        }
        
	string type = nodeElement->get_attribute_value("type");
        string pathway = nodeElement->get_attribute_value("pathways");        
        
	string display_names;
	
	// For nodes that are complexes fill components
	vector<int> components_list;//contains only the xml file ids of the component nodes for now
	components_list = get_component_list_for_xml_node(node);


	//if the nodes is not a complex(group)
	if(components_list.empty()){
		set<string> rep_ids_set;//set of rep ids that are there in this node
		for(set<string>::iterator all_id_itr = all_ids.begin(); all_id_itr != all_ids.end(); all_id_itr++){
			//find the current id in the all id map
			//map<string, string>::iterator all_id_map_itr = (graph_man->all_id_map).find(*all_id_itr);
                                            
			string repid = graph_man->get_rep_id_for_id(*all_id_itr);
                        
			if(repid == ""){
                            	//if the id is not in the all id map then id itself becomes rep id and also inserts this info in all id map
				//think of compounds for this case
				//graph_man->add_rep_id_for_id(*all_id_itr, *all_id_itr);
                                // rep_ids_set.insert(*all_id_itr);
                                if (!split_node) {
                                    rep_ids_set.insert(all_rep_ids_str);
                                    //graph_man->add_rep_id_for_id(*all_id_itr, all_rep_ids_str);
                                    graph_ptr->add_id_rep_id(*all_id_itr, all_rep_ids_str);
                                }
                                else {
                                    rep_ids_set.insert(*all_id_itr);
                                    //graph_man->add_rep_id_for_id(*all_id_itr, *all_id_itr);
                                    graph_ptr->add_id_rep_id(*all_id_itr, *all_id_itr);
                                }
                                
                            
			}
			else{
				rep_ids_set.insert(repid);
			}
		}

		for(set<string>::iterator rid_itr = rep_ids_set.begin(); rid_itr != rep_ids_set.end(); rid_itr++){
			int nid = graph_ptr->get_nid_from_rep_id(*rid_itr);
                        
                        if(nid == -1){//nid == -1 means repid not there in the graph as of now
				//nid = ++GraphManagerNew::node_id_count;
                                nid = graph_ptr->create_new_node();
                                graph_ptr->add_node_id(nid);
                                graph_ptr->add_rep_id_for_node(nid, *rid_itr);
                                graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
				//vector<string> all_id_for_rid = graph_man->get_all_ids_for_rep_id(*rid_itr); // changed by sukanya on 25 July 2016
                                ////vector<string> all_id_for_rid = graph_ptr->get_all_ids_for_rep_id(*rid_itr);
                                vector<string> all_id_for_rid = all_ids_vec;
                                for(vector<string>::iterator all_id_for_rid_itr = all_id_for_rid.begin(); all_id_for_rid_itr != all_id_for_rid.end(); all_id_for_rid_itr++){
                                    
					graph_ptr->add_id_for_node(nid, *all_id_for_rid_itr);
					
                                        //graph_ptr->add_id_rep_id(*all_id_for_rid_itr, *rid_itr);
                                        if (!split_node) {
                                            graph_ptr->add_id_rep_id(*all_id_for_rid_itr, all_rep_ids_str);
                                            graph_ptr->add_id_rep_id(all_rep_ids_str, all_rep_ids_str);
                                            
                                            if (!graph_ptr->node_has_id(nid, all_rep_ids_str))
                                                graph_ptr->add_id_for_node(nid, all_rep_ids_str);
                                            
                                        }
                                        else {
                                            graph_ptr->add_id_rep_id(*all_id_for_rid_itr, *rid_itr);
                                        }
                                        
				}
//                                graph_ptr->add_id_for_node(nid, all_rep_ids_str);
//                                graph_ptr->add_id_rep_id(all_rep_ids_str, all_rep_ids_str);
                                
				graph_ptr->add_node_type(nid, type);
                                // added by sukanya on 1 Oct 2015
                                
                                display_names = graph_man->get_display_names_from_rep_id(*rid_itr);
                                list<string> all_display_names;
                                split_string_into_list(display_names, ",", all_display_names);
				for(list<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
					graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
				}
			}
			else{
                            	xmlpp::Node::NodeList node_subtype_list1 = nodeElement->get_children("graphics");
                            	for(xmlpp::Node::NodeList::iterator node_subtype_iter1 = node_subtype_list1.begin(); node_subtype_iter1 != node_subtype_list1.end(); ++node_subtype_iter1) {
                            		const xmlpp::Element* nodeSubtypeElement1 = dynamic_cast<const xmlpp::Element*>(*node_subtype_iter1);
                            		display_names = nodeSubtypeElement1->get_attribute_value("name");
                            	}
                            	list<string> all_display_names;
                                split_string_into_list(display_names, ", ", all_display_names);
				//if the new display names are not there add them
				for(list<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
					if(!graph_ptr->node_has_display_id(nid, *dis_name_itr)){
						graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
					}
				}
			}

			nodes_set.insert(nid);
		}
	}
	else{
            //if the node is a complex(group)
		vector<set<int> > components_set;
		//cycle detection in components already done
		for(vector<int>::iterator cl_itr = components_list.begin(); cl_itr != components_list.end(); cl_itr++){
			set<int> comp_member_node_set = process_node(graph_ptr, file_id_node_map.find(*cl_itr)->second, graph_man, file_id_node_map, thres);
                        cartestian_product(components_set, comp_member_node_set);

		}

                
		for(vector<set<int> >::iterator vec_set_itr = components_set.begin(); vec_set_itr != components_set.end(); vec_set_itr++){
                    
			int nid = graph_ptr->get_node_id_with_components(*vec_set_itr);
			if (nid == -1){
				//nid = ++GraphManagerNew::node_id_count;
                                nid = graph_ptr->create_new_node();
				graph_ptr->add_node_id(nid);
				graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
				string complex_id = graph_ptr->concatenate_sorted_rep_ids(*vec_set_itr, "_");
                                complex_id = "c_s_" + complex_id + "_c_e";
                                
                                // -- added by sukanya on 7 Aug 2016
                                set<int> nid_set_of_complex = *vec_set_itr;
                                vector<string> comp_disp_names; // used vector to retain the order of nodes
                                for (auto nid_set_itr = nid_set_of_complex.begin(); nid_set_itr != nid_set_of_complex.end(); nid_set_itr++) {
                                    comp_disp_names.push_back(graph_ptr->get_all_display_ids_of_node(*nid_set_itr)[0]);
                                }
                                string complex_disp_id = concatenate_strings(comp_disp_names, "+");
                                complex_disp_id = "[" + complex_disp_id + "]";
                                graph_ptr->add_display_id_for_node(nid, complex_disp_id);
                                // --
                                
				graph_man->add_rep_id_for_id(complex_id, complex_id);
				graph_ptr->add_id_for_node(nid, complex_id);
                                graph_ptr->add_rep_id_for_node(nid, complex_id);
				graph_ptr->add_id_rep_id(complex_id, complex_id);
				graph_ptr->add_node_type(nid, type);
				for(set<int>::iterator comp_mem_itr = vec_set_itr->begin(); comp_mem_itr != vec_set_itr->end(); comp_mem_itr++){
					graph_ptr->add_component_id_for_node(nid, *comp_mem_itr);
				}
				//graph_ptr->add_display_id_for_node(nid, complex_id);
                                
			}
			nodes_set.insert(nid);
		}
	}

	return nodes_set;
}



/*
std::set<int> XMLParser::process_node(GraphNew* graph_ptr, xmlpp::Node* node, GraphManagerNew* graph_man, std::map<int, xmlpp::Node*>& file_id_node_map){
	std::set<int> nodes_set;
	const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);

	//get all the labels for the node
	string name = nodeElement->get_attribute_value("name"); // hsa ids in xml file
	// Erase invalid dot characters from id name
	char chars[] = ":";
	for (unsigned int i = 0; i < strlen(chars); ++i) {
		name.erase (std::remove(name.begin(), name.end(), chars[i]), name.end());
	}

	set<string> all_ids = split_string(name, " ");

	string type = nodeElement->get_attribute_value("type");

	string display_names;
	// get display names
	xmlpp::Node::NodeList node_subtype_list1 = nodeElement->get_children("graphics");
	for(xmlpp::Node::NodeList::iterator node_subtype_iter1 = node_subtype_list1.begin(); node_subtype_iter1 != node_subtype_list1.end(); ++node_subtype_iter1) {
		const xmlpp::Element* nodeSubtypeElement1 = dynamic_cast<const xmlpp::Element*>(*node_subtype_iter1);
		display_names = nodeSubtypeElement1->get_attribute_value("name");
	}
	set<string> all_display_names = split_string(display_names, ", ");

	// For nodes that are complexes fill components
	vector<int> components_list;//contains only the xml file ids of the component nodes for now
	components_list = get_component_list_for_xml_node(node);


	//if the nodes is not a complex(group)
	if(components_list.empty()){
		set<string> rep_ids_set;//set of rep ids that are there in this node
		for(set<string>::iterator all_id_itr = all_ids.begin(); all_id_itr != all_ids.end(); all_id_itr++){
			//find the current id in the all id map
			//map<string, string>::iterator all_id_map_itr = graph_man->all_id_map.find(*all_id_itr);
			string repid = graph_man->get_rep_id_for_id(*all_id_itr);
			if(repid == ""){
				//if the id is not in the all id map then id itself becomes rep id and also inserts this info in all id map
				//think of compounds for this case
				//graph_man->all_id_map[*all_id_itr] = *all_id_itr;
				graph_man->add_rep_id_for_id(*all_id_itr, *all_id_itr);
				rep_ids_set.insert(*all_id_itr);
			}
			else{
				rep_ids_set.insert(repid);
			}
		}

		for(set<string>::iterator rid_itr = rep_ids_set.begin(); rid_itr != rep_ids_set.end(); rid_itr++){
			int nid = graph_ptr->get_nid_from_rep_id(*rid_itr);
			if(nid == -1){//nid == -1 means repid not there in the graph as of now
				//nid = ++GraphManagerNew::node_id_count;
                                nid = graph_ptr->create_new_node(); // creates a new node -- added by sukanya
                                graph_ptr->add_node_id(nid);
                                graph_ptr->add_rep_id_for_node(nid, *rid_itr); // added by sukanya
                                graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
				vector<string> all_id_for_rid = graph_man->get_all_ids_for_rep_id(*rid_itr);
				for(vector<string>::iterator all_id_for_rid_itr = all_id_for_rid.begin(); all_id_for_rid_itr != all_id_for_rid.end(); all_id_for_rid_itr++){
					graph_ptr->add_id_for_node(nid, *all_id_for_rid_itr);
					graph_ptr->add_id_rep_id(*all_id_for_rid_itr, *rid_itr); // commented out by sukanya
				}
				graph_ptr->add_node_type(nid, type);
				for(set<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
					graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
				}
			}
			else{
				//if the new display names are not there add them
				for(set<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
					if(!graph_ptr->node_has_display_id(nid, *dis_name_itr)){
						graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
					}
				}
			}

			nodes_set.insert(nid);
		}
	}
	else{//if the node is a complex(group)
		vector<set<int> > components_set;
		//cycle detection in components already done
		for(vector<int>::iterator cl_itr = components_list.begin(); cl_itr != components_list.end(); cl_itr++){
			set<int> comp_member_node_set = process_node(graph_ptr, file_id_node_map.find(*cl_itr)->second, graph_man, file_id_node_map);
			cartestian_product(components_set, comp_member_node_set);
		}

		for(vector<set<int> >::iterator vec_set_itr = components_set.begin(); vec_set_itr != components_set.end(); vec_set_itr++){
			int nid = graph_ptr->get_node_id_with_components(*vec_set_itr);
			if (nid == -1){
				//nid = ++GraphManagerNew::node_id_count;
                                nid = graph_ptr->create_new_node(); // creates a new node -- added by sukanya
				graph_ptr->add_node_id(nid);
				graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
				string complex_id = graph_ptr->concatenate_sorted_rep_ids(*vec_set_itr, "_");
				complex_id = "c_s_" + complex_id + "_c_e";
				graph_man->add_rep_id_for_id(complex_id, complex_id);
				graph_ptr->add_id_for_node(nid, complex_id);
                                graph_ptr->add_rep_id_for_node(nid, complex_id); // added by sukanya
				graph_ptr->add_id_rep_id(complex_id, complex_id); // commented out by sukanya
				graph_ptr->add_node_type(nid, type);
				for(set<int>::iterator comp_mem_itr = vec_set_itr->begin(); comp_mem_itr != vec_set_itr->end(); comp_mem_itr++){
					graph_ptr->add_component_id_for_node(nid, *comp_mem_itr);
				}
				graph_ptr->add_display_id_for_node(nid, complex_id);
			}
			nodes_set.insert(nid);
		}
	}

	return nodes_set;
}*/

void XMLParser::process_edge(GraphNew* graph_ptr, int source_node_id, int target_node_id, xmlpp::Node* edge, GraphManagerNew* graph_man, string pathway, bool tool_written){
	const xmlpp::Element* edgeElement = dynamic_cast<const xmlpp::Element*>(edge);
	const xmlpp::Element::AttributeList& edge_attr_list = edgeElement->get_attributes();
	string type = edgeElement->get_attribute_value("type");
	vector<string> subtype;

	xmlpp::Node::NodeList edge_subtype_list = edgeElement->get_children("subtype");
	bool is_undirected = false;
	for(xmlpp::Node::NodeList::iterator edge_subtype_iter = edge_subtype_list.begin(); edge_subtype_iter != edge_subtype_list.end(); ++edge_subtype_iter) {
		// Get attributes of subtype. eg., activation, inhibition, etc.
		const xmlpp::Element* edgeSubtypeElement = dynamic_cast<const xmlpp::Element*>(*edge_subtype_iter);
		string curr_subtype = edgeSubtypeElement->get_attribute_value("name");
		if(curr_subtype == "indirect effect"){
			curr_subtype = "indirect";
		}
		if(curr_subtype == "missing interaction"){
			curr_subtype = "missing";
		}
		if(curr_subtype == "state change"){
			curr_subtype = "state";
		}
		if(curr_subtype == "binding/association"){
			curr_subtype = "association";
		}
		subtype.push_back(curr_subtype);
		if(curr_subtype == "association" || curr_subtype == "dissociation" || curr_subtype == "mapping"){
			is_undirected = true;
		}
	}

        
	int eid = tool_written ? -1 : graph_ptr->check_if_edge_already_created(source_node_id, target_node_id, type, subtype);
        
#ifdef DUMMY_EDGE_FLAG                       
        if ((!tool_written) && (eid == -1)) {
            vector<string> subtype_copy = subtype;
            subtype_copy.push_back("dummy_u_to_d");
            eid = graph_ptr->check_if_edge_already_created(source_node_id, target_node_id, type, subtype_copy);
        }
#endif  
        
	if( eid == -1){
		//eid = ++GraphManagerNew::edge_id_count;
                eid = graph_ptr->create_new_edge(); // creates a new edge -- added by sukanya
                graph_ptr->add_edge_id(eid); // added by sukanya
		graph_man->add_edge_id_graph_id(eid, graph_ptr->get_graph_id());
		graph_ptr->add_edge_type(eid, type);
		graph_ptr->add_edge_to_outlist_of_node(source_node_id, eid);
		graph_ptr->add_edge_to_inlist_of_node(target_node_id, eid);
		graph_ptr->add_source_node(eid, source_node_id);
		graph_ptr->add_target_node(eid, target_node_id);
                
		for(vector<string>::iterator subt_itr = subtype.begin(); subt_itr != subtype.end(); subt_itr++){
			graph_ptr->add_subtype_for_edge(eid, *subt_itr);
		}
                
                
#ifdef DUMMY_EDGE_FLAG
                if(is_undirected && !tool_written){
                        if (graph_ptr->get_rep_id_from_nid(source_node_id) > graph_ptr->get_rep_id_from_nid(target_node_id))
                                graph_ptr->add_subtype_for_edge(eid, "dummy_u_to_d");
                }
#endif                

		if(tool_written){
			string all_pathways_str = edgeElement->get_attribute_value("pathways");
			//cout << all_pathways_str << endl;
			set<string> all_pathway_vec = split_string(all_pathways_str, " ");
                        
			for(set<string>::iterator p_itr = all_pathway_vec.begin(); p_itr != all_pathway_vec.end(); p_itr++){
				graph_ptr->add_pathway_for_edge(eid, *p_itr);
			}
		}
		else{
			graph_ptr->add_pathway_for_edge(eid, pathway);
		}
	}

	//the undirected case of assoc and dissoc needs edges in both directions in our representation
        //if(!tool_written){
	//if(is_undirected){
        //if(is_undirected && (source_node_id < target_node_id)){
        if(is_undirected && !tool_written){
            
		//int eid = tool_written ? -1 : graph_ptr->check_if_edge_already_created(target_node_id, source_node_id, type, subtype);
                int eid = graph_ptr->check_if_edge_already_created(target_node_id, source_node_id, type, subtype);
#ifdef DUMMY_EDGE_FLAG                       
                if ((!tool_written) && (eid == -1)) {
                    vector<string> subtype_copy = subtype;
                    subtype_copy.push_back("dummy_u_to_d");
                    eid = graph_ptr->check_if_edge_already_created(target_node_id, source_node_id, type, subtype_copy);
                }
#endif            
		if( eid == -1){
			//eid = ++GraphManagerNew::edge_id_count;
                        eid = graph_ptr->create_new_edge(); // creates a new edge -- added by sukanya
			graph_ptr->add_edge_id(eid);
			graph_man->add_edge_id_graph_id(eid, graph_ptr->get_graph_id());
			graph_ptr->add_edge_type(eid, type);
			graph_ptr->add_edge_to_outlist_of_node(target_node_id, eid);
			graph_ptr->add_edge_to_inlist_of_node(source_node_id, eid);
			graph_ptr->add_source_node(eid, target_node_id);
			graph_ptr->add_target_node(eid, source_node_id);
                        
                        for(vector<string>::iterator subt_itr = subtype.begin(); subt_itr != subtype.end(); subt_itr++){
				graph_ptr->add_subtype_for_edge(eid, *subt_itr);
 			}
#ifdef DUMMY_EDGE_FLAG
                        if (graph_ptr->get_rep_id_from_nid(target_node_id) > graph_ptr->get_rep_id_from_nid(source_node_id))
                                graph_ptr->add_subtype_for_edge(eid, "dummy_u_to_d");
#endif

                        
//			if(tool_written){
//				string all_pathways_str = edgeElement->get_attribute_value("pathways");
//				set<string> all_pathway_vec = split_string(all_pathways_str, " ");
//				for(set<string>::iterator p_itr = all_pathway_vec.begin(); p_itr != all_pathway_vec.end(); p_itr++){
//					graph_ptr->add_pathway_for_edge(eid, *p_itr);
//				}
//			}
//			else{
				graph_ptr->add_pathway_for_edge(eid, pathway);
//			}
		}
	}
}

//////void SBMLParser::process_edge_sbml(GraphNew* graph_ptr, int source_node_id, int target_node_id, const Reaction * species, GraphManagerNew* graph_man, string pathway, bool tool_written){
//////	
//////    string type = "reaction";
//////    vector<string> subtype;
//////    subtype.push_back("reaction");
//////    
//////	int eid = tool_written ? -1 : graph_ptr->check_if_edge_already_created(source_node_id, target_node_id, "reaction", subtype);
//////        
//////	if( eid == -1){
//////		//eid = ++GraphManagerNew::edge_id_count;
//////                eid = graph_ptr->create_new_edge(); // creates a new edge -- added by sukanya
//////                graph_ptr->add_edge_id(eid); // added by sukanya
//////		graph_man->add_edge_id_graph_id(eid, graph_ptr->get_graph_id());
//////		graph_ptr->add_edge_type(eid, type);
//////		graph_ptr->add_edge_to_outlist_of_node(source_node_id, eid);
//////		graph_ptr->add_edge_to_inlist_of_node(target_node_id, eid);
//////		graph_ptr->add_source_node(eid, source_node_id);
//////		graph_ptr->add_target_node(eid, target_node_id);
//////		for(vector<string>::iterator subt_itr = subtype.begin(); subt_itr != subtype.end(); subt_itr++){
//////			graph_ptr->add_subtype_for_edge(eid, *subt_itr);
//////		}
//////                
//////
////////		if(tool_written){
//////			//string all_pathways_str = edgeElement->get_attribute_value("pathways");
//////                        //string pathway_str = species->getModel()->getName();
//////                        string pathway_str = pathway;
//////                        graph_ptr->add_pathway_for_edge(eid, pathway_str);
//////                        
////////			//cout << all_pathways_str << endl;
////////			set<string> all_pathway_vec = split_string(all_pathways_str, " ");
////////			for(set<string>::iterator p_itr = all_pathway_vec.begin(); p_itr != all_pathway_vec.end(); p_itr++){
////////				graph_ptr->add_pathway_for_edge(eid, *p_itr);
////////			}
////////		}
////////		else{
////////			graph_ptr->add_pathway_for_edge(eid, pathway);
////////		}
//////	}
//////////
//////////	//the undirected case of assoc and dissoc needs edges in both directions in our representation
//////////        //if(!tool_written){
//////////	//if(is_undirected){
//////////        //if(is_undirected && (source_node_id < target_node_id)){
//////////        if(is_undirected && !tool_written){
//////////            
//////////		//int eid = tool_written ? -1 : graph_ptr->check_if_edge_already_created(target_node_id, source_node_id, type, subtype);
//////////                int eid = graph_ptr->check_if_edge_already_created(target_node_id, source_node_id, type, subtype);
//////////            
//////////		if( eid == -1){
//////////			//eid = ++GraphManagerNew::edge_id_count;
//////////                        eid = graph_ptr->create_new_edge(); // creates a new edge -- added by sukanya
//////////			graph_ptr->add_edge_id(eid);
//////////			graph_man->add_edge_id_graph_id(eid, graph_ptr->get_graph_id());
//////////			graph_ptr->add_edge_type(eid, type);
//////////			graph_ptr->add_edge_to_outlist_of_node(target_node_id, eid);
//////////			graph_ptr->add_edge_to_inlist_of_node(source_node_id, eid);
//////////			graph_ptr->add_source_node(eid, target_node_id);
//////////			graph_ptr->add_target_node(eid, source_node_id);
//////////			for(vector<string>::iterator subt_itr = subtype.begin(); subt_itr != subtype.end(); subt_itr++){
//////////				graph_ptr->add_subtype_for_edge(eid, *subt_itr);
//////////			}
//////////
//////////			if(tool_written){
//////////				string all_pathways_str = edgeElement->get_attribute_value("pathways");
//////////				set<string> all_pathway_vec = split_string(all_pathways_str, " ");
//////////				for(set<string>::iterator p_itr = all_pathway_vec.begin(); p_itr != all_pathway_vec.end(); p_itr++){
//////////					graph_ptr->add_pathway_for_edge(eid, *p_itr);
//////////				}
//////////			}
//////////			else{
//////////				graph_ptr->add_pathway_for_edge(eid, pathway);
//////////			}
//////////		}
//////////	}
////////}
//////}

/*
void XMLParser::fill_graph_from_xml_file_as_it_is(GraphNew* graph_ptr, string input_file, GraphManagerNew* graph_man){
	//parse the xml flie
	xmlpp::DomParser parser;
	parser.parse_file(input_file);
	const xmlpp::Node* pNode = parser.get_document()->get_root_node();
	const xmlpp::Element* rootElement = dynamic_cast<const xmlpp::Element*>(pNode);

	string pathway = rootElement->get_attribute_value("title");

	//get all the nodes from the xml file
	// For nodes in XML file
	xmlpp::Node::NodeList nodes_list = pNode->get_children("entry");
	assert(!nodes_list.empty());

	//create a map of file node id (as in XML file ) to node for quick access to source and target nodes while iterating over the edges
	std::map<int, xmlpp::Node*> file_id_node_map;
	for(xmlpp::Node::NodeList::iterator iter = nodes_list.begin(); iter != nodes_list.end(); ++iter) {
		const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(*iter);
		int id = atoi(nodeElement->get_attribute_value("id").c_str());
		file_id_node_map[id] = *iter;
	}
	assert(!file_id_node_map.empty());

	if(cycle_in_components_relations(file_id_node_map)){
		cerr << "Error: cycle detected in components relation in the graph: " + input_file << endl;
		graph_man->destroy_graph(graph_ptr);
		return;
	}

	//now iterate over each edge and fill graph
	xmlpp::Node::NodeList edges_list = pNode->get_children("relation");
	for(xmlpp::Node::NodeList::iterator iter = edges_list.begin(); iter != edges_list.end(); ++iter) {
		const xmlpp::Element* edgeElement = dynamic_cast<const xmlpp::Element*>(*iter);
		int file_id_source = atoi(edgeElement->get_attribute_value("entry1").c_str());
		int file_id_target = atoi(edgeElement->get_attribute_value("entry2").c_str());


		int source_node_id = process_node_as_it_is(graph_ptr, file_id_source, graph_man, file_id_node_map);
		int target_node_id = process_node_as_it_is(graph_ptr,file_id_target, graph_man, file_id_node_map);

		process_edge(graph_ptr, source_node_id, target_node_id, *iter, graph_man, pathway);
	}
}
*/

/*
int XMLParser::process_node_as_it_is(GraphNew* graph_ptr, int node_id_in_file, GraphManagerNew* graph_man, std::map<int, xmlpp::Node*>& file_id_node_map){
	int nid;
	xmlpp::Node* node = file_id_node_map[node_id_in_file];
	const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);

	//get all the labels for the node
	string name = nodeElement->get_attribute_value("name"); // hsa ids in xml file
	// Erase invalid dot characters from id name
	char chars[] = ":";
	for (unsigned int i = 0; i < strlen(chars); ++i) {
		name.erase (std::remove(name.begin(), name.end(), chars[i]), name.end());
	}

	set<string> all_ids = split_string(name, " ");

	string type = nodeElement->get_attribute_value("type");

	string display_names;
	// get display names
	xmlpp::Node::NodeList node_subtype_list1 = nodeElement->get_children("graphics");
	for(xmlpp::Node::NodeList::iterator node_subtype_iter1 = node_subtype_list1.begin(); node_subtype_iter1 != node_subtype_list1.end(); ++node_subtype_iter1) {
		const xmlpp::Element* nodeSubtypeElement1 = dynamic_cast<const xmlpp::Element*>(*node_subtype_iter1);
		display_names = nodeSubtypeElement1->get_attribute_value("name");
	}
	set<string> all_display_names = split_string(display_names, ", ");

	// For nodes that are complexes fill components
	vector<int> components_list;//contains only the xml file ids of the component nodes for now
	components_list = get_component_list_for_xml_node(node);

	//here the rep id is the id of the node in the xml file
	stringstream ss;
	ss << node_id_in_file;
	string repid = ss.str();
	nid = graph_ptr->get_nid_from_rep_id(repid);
	if( nid != -1){
		//if graph already has this rep id then return that node id
		return nid;
	}
	else{
		nid = ++GraphManagerNew::node_id_count;
		graph_ptr->add_node_id(nid);
		graph_man->add_node_id_graph_id(nid, graph_ptr->get_graph_id());
		graph_ptr->add_node_type(nid, type);
		//if the node is not a complex(group)
		if(components_list.empty()){
			for(set<string>::iterator all_id_itr = all_ids.begin(); all_id_itr != all_ids.end(); all_id_itr++){
				graph_ptr->add_id_for_node(nid, *all_id_itr);
				graph_ptr->add_id_rep_id(*all_id_itr, repid);
			}
			for(set<string>::iterator dis_name_itr = all_display_names.begin(); dis_name_itr != all_display_names.end(); dis_name_itr++){
				graph_ptr->add_display_id_for_node(nid, *dis_name_itr);
			}
		}
		else{//if the node is a complex(group)
			set<int> components_set;
			//cycle detection in components already done
			for(vector<int>::iterator cl_itr = components_list.begin(); cl_itr != components_list.end(); cl_itr++){
				int comp_member_node_id = process_node_as_it_is(graph_ptr, file_id_node_map.find(*cl_itr)->second, graph_man, file_id_node_map);
				components_set.insert(comp_member_node_id);
			}

			string complex_id = graph_ptr->concatenate_sorted_rep_ids(components_set, "_");
			complex_id = "c_s_" + complex_id + "_c_e";
			graph_ptr->add_id_for_node(nid, complex_id);
			graph_ptr->add_id_rep_id(complex_id, complex_id);
			for(set<int>::iterator comp_mem_itr = components_set.begin(); comp_mem_itr != components_set.end(); comp_mem_itr++){
				graph_ptr->add_component_id_for_node(nid, *comp_mem_itr);
			}
			graph_ptr->add_display_id_for_node(nid, complex_id);
		}
	}
	return nid;
}
*/

//the xml file should have complex(group) enties after non-complex entries
map<string, Node *> XMLParser::createGraphFromXMLNew(GraphManager *gm, string inputfilename) {
	map<string, Node *> new_graph;
	map<int, string> node_id_map_graph;//this stores mapping from local "id" of xml file to rep id
	map<int, Node*> node_id_map;////this stores mapping from local "id" to Node*

	//parse the xml flie
	xmlpp::DomParser parser;
	parser.parse_file(inputfilename);
	const xmlpp::Node* pNode = parser.get_document()->get_root_node();

	//get all the nodes from the xml file
	// For nodes in XML file
	xmlpp::Node::NodeList nodes_list = pNode->get_children("entry");

	//update the equivalence class map for nodes
	for(xmlpp::Node::NodeList::iterator iter = nodes_list.begin(); iter != nodes_list.end(); ++iter) {
		const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(*iter);
		string name = nodeElement->get_attribute_value("name");
		int num = atoi(nodeElement->get_attribute_value("id").c_str());
		string type = nodeElement->get_attribute_value("type");

		// Erase invalid dot characters from id name
		char chars[] = ":";
		for (unsigned int i = 0; i < strlen(chars); ++i) {
			name.erase (std::remove(name.begin(), name.end(), chars[i]), name.end());
		}

		string name_delim = " ";

		//if type is a complex of proteins (group) then its rep id must be constructed using node_id_map_graph
		if(type == "group"){
			//get all local ids of the component
			// Get ids of components of node
			vector<string> comp_ids;
			xmlpp::Node::NodeList node_comp_list = nodeElement->get_children("component");
			for(xmlpp::Node::NodeList::iterator comp_iter = node_comp_list.begin(); comp_iter != node_comp_list.end(); comp_iter++) {
				const xmlpp::Element* compElement = dynamic_cast<const xmlpp::Element*>(*comp_iter);
				string comp_id = compElement->get_attribute_value("id");
				int comp_id_int = atoi(comp_id.c_str());

				map<int, string>::iterator node_id_map_itr = node_id_map_graph.find(comp_id_int);

				if(node_id_map_itr == node_id_map_graph.end()){
					cerr << "\nthe id included in the complex is not present in the graph, exiting ...\n";
					exit(1);
				}
				else{
					//get the repid of the component to make rep id of the complex
					comp_ids.push_back(node_id_map_itr->second);
				}
			}
			//sorting needed for consistent ordering
			sort(comp_ids.begin(), comp_ids.end());
			string rep_id_group = "c";
			for(vector<string>::iterator comp_id_itr = comp_ids.begin(); comp_id_itr != comp_ids.end(); ++comp_id_itr){
				rep_id_group += "_" + *comp_id_itr;
			}

			//so we are ready with the rep id of the node for complex
			node_id_map_graph[num] = rep_id_group;
			gm->id_equiv_class_map[rep_id_group] = rep_id_group;

		}

		else{//for non groups
			//get all ids (e.g. hsa100 hsa200 hsa300) for the current node
			list<string> *pList_of_names = gm->get_list_of_names_from_string(name, name_delim);
			assert(!pList_of_names->empty());

			bool any_id_alreay_exists = false;
			string rep_id;

			for (list<string>::iterator list_it = pList_of_names->begin(); list_it != pList_of_names->end(); list_it++) {
				for (map<string, string>::iterator map_itr = gm->id_equiv_class_map.begin(); map_itr != gm->id_equiv_class_map.end(); map_itr++) {
					if ((*list_it) == map_itr->first) {
						any_id_alreay_exists = true;
						rep_id = map_itr->second;
						break;
					}
				}
				if (any_id_alreay_exists)
					break;
			}

			//if any of these ids already exist in the map then make all the ids map to the appropriate rep id
			if(any_id_alreay_exists){
				for (list<string>::iterator list_it = pList_of_names->begin(); list_it != pList_of_names->end(); list_it++) {
					gm->id_equiv_class_map[*list_it] = rep_id;
				}
				node_id_map_graph[num] = rep_id;
			}
			else{//else make the first id the new rep if for all of the other ids and insert in the equiv class map
				rep_id = *pList_of_names->begin();
				for (list<string>::iterator list_it = pList_of_names->begin(); list_it != pList_of_names->end(); list_it++) {
					gm->id_equiv_class_map[*list_it] = rep_id;
				}
				node_id_map_graph[num] = rep_id;
			}

			//flatten the map that stores equivalence class
			gm->flatten_id_name_map();
		}
	}

	//now once the map for equivalence class of ids is ready create nodes for nodes that are not complexes
	for(xmlpp::Node::NodeList::iterator iter = nodes_list.begin(); iter != nodes_list.end(); ++iter) {
		int num;
		string name, type, link = "";
		const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(*iter);
		num = atoi(nodeElement->get_attribute_value("id").c_str());
		name = nodeElement->get_attribute_value("name"); // hsa ids in xml file
		type = nodeElement->get_attribute_value("type");
		link = nodeElement->get_attribute_value("link");
		string node_display_name;
		string name_delim = " ";
		string display_delim = ", ";

		// Find display properties of node
		xmlpp::Node::NodeList node_subtype_list1 = nodeElement->get_children("graphics");
		for(xmlpp::Node::NodeList::iterator node_subtype_iter1 = node_subtype_list1.begin(); node_subtype_iter1 != node_subtype_list1.end(); ++node_subtype_iter1) {
			const xmlpp::Element* nodeSubtypeElement1 = dynamic_cast<const xmlpp::Element*>(*node_subtype_iter1);
			node_display_name = nodeSubtypeElement1->get_attribute_value("name");
		}

		// Erase invalid dot characters from id name
		char chars[] = ":";
		for (unsigned int i = 0; i < strlen(chars); ++i) {
			name.erase (std::remove(name.begin(), name.end(), chars[i]), name.end());
		}

		// Erase invalid dot characters from display name
		char chars2[] = "-.:'";
		for (unsigned int i = 0; i < strlen(chars2); ++i) {
			node_display_name.erase (std::remove(node_display_name.begin(), node_display_name.end(), chars2[i]), node_display_name.end());
		}

		list<string> * pList_of_display_names = gm->get_list_of_names_from_string(node_display_name, display_delim);
		assert(!pList_of_display_names->empty());
		//pick the first display name for simplicity
		string rep_display_name = *pList_of_display_names->begin();

		string rep_id_name = node_id_map_graph.find(num)->second;

		// For nodes that are complexes
		vector<Node*> comp_set;

		if(type == "group"){
			//get all local ids of the component
			// Get ids of components of node
			vector<string> comp_ids;
			xmlpp::Node::NodeList node_comp_list = nodeElement->get_children("component");
			for(xmlpp::Node::NodeList::iterator comp_iter = node_comp_list.begin(); comp_iter != node_comp_list.end(); comp_iter++) {
				const xmlpp::Element* compElement = dynamic_cast<const xmlpp::Element*>(*comp_iter);
				string comp_id = compElement->get_attribute_value("id");
				int comp_id_int = atoi(comp_id.c_str());

				map<int, Node*>::iterator node_id_map_itr = node_id_map.find(comp_id_int);
				if(node_id_map_itr == node_id_map.end()){
					cerr << "\ncomponent id int group missing from map, exiting .. " << endl;
					exit(1);
				}
				comp_set.push_back(node_id_map_itr->second);
			}

		}

		Node* new_node;
		map<string, Node*>::iterator new_graph_itr = new_graph.find(rep_id_name);
		//create new node iff the node for this repid already doesn't exist in the graph map
		if(new_graph_itr == new_graph.end()){
			new_node = gm->create_node(rep_id_name, rep_display_name, comp_set, type, link);
			new_graph.insert(pair<string ,Node *>(rep_id_name, new_node));
			gm->add_to_local_to_hsa_node_id_map(new_node->node_id, new_node->rep_id_name);
		}else{
			new_node = new_graph_itr->second;
		}

		node_id_map.insert(pair<int,Node *>(num, new_node));
	}

	//for edges i.e. realtions in the XML file


	//print_map(gm->id_equiv_class_map);

	exit(1);

	return new_graph;
}


map<string, Node *> XMLParser::createGraphFromXML(GraphManager *gm, string inputfilename) {
	xmlpp::DomParser parser;
		
	map<string, Node *> new_graph; // finally returned
	
	map<int, Node*> node_id_map;
	//map<unsigned int, char> edge_map;
	set<bvatom> edge_map;
	
	parser.parse_file(inputfilename);
  
	const xmlpp::Node* pNode = parser.get_document()->get_root_node();
		
	string p_name, p_title;
	const xmlpp::Element* rootElement = dynamic_cast<const xmlpp::Element*>(pNode);
	p_name = rootElement->get_attribute_value("name");
	p_title = rootElement->get_attribute_value("title");
	

	list<string> pathways;
    pathways.push_back(p_name);
                
	// For nodes in XML file
	xmlpp::Node::NodeList nodes_list = pNode->get_children("entry");
  
	for(xmlpp::Node::NodeList::iterator iter = nodes_list.begin(); iter != nodes_list.end(); ++iter) {
		int num;
		string name, type, link = "";
		const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(*iter);
		num = atoi(nodeElement->get_attribute_value("id").c_str());
		name = nodeElement->get_attribute_value("name"); // hsa ids in xml file
		type = nodeElement->get_attribute_value("type");
		link = nodeElement->get_attribute_value("link");
		string node_display_name;
				
		  
		// Find display properties of node
		xmlpp::Node::NodeList node_subtype_list1 = nodeElement->get_children("graphics");
		for(xmlpp::Node::NodeList::iterator node_subtype_iter1 = node_subtype_list1.begin(); node_subtype_iter1 != node_subtype_list1.end(); ++node_subtype_iter1) {
			const xmlpp::Element* nodeSubtypeElement1 = dynamic_cast<const xmlpp::Element*>(*node_subtype_iter1);
			node_display_name = nodeSubtypeElement1->get_attribute_value("name");
			
				
		}
		
		// Erase invalid dot characters from id name
		char chars[] = ":";
		for (unsigned int i = 0; i < strlen(chars); ++i) {
                    	name.erase (std::remove(name.begin(), name.end(), chars[i]), name.end());
		}
		
		// Erase invalid dot characters from display name
		char chars2[] = "-.:'";
		for (unsigned int i = 0; i < strlen(chars2); ++i) {
                    	node_display_name.erase (std::remove(node_display_name.begin(), node_display_name.end(), chars2[i]), node_display_name.end());
		}
		
		
		// For nodes that are complexes
		vector<Node*> comp_set;
		if (type == "group") {

			set <string> comp_ids;
			
			// Get ids of components of node
			xmlpp::Node::NodeList node_comp_list = nodeElement->get_children("component");
			int num_of_components = 0;
			for(xmlpp::Node::NodeList::iterator comp_iter = node_comp_list.begin(); comp_iter != node_comp_list.end(); comp_iter++) {
				const xmlpp::Element* compElement = dynamic_cast<const xmlpp::Element*>(*comp_iter);
				string comp_id = compElement->get_attribute_value("id");
				
				// Get component node
				if (node_id_map.find(atoi(comp_id.c_str())) != node_id_map.end()) {
					
					Node * comp_node = (node_id_map.find(atoi(comp_id.c_str())))->second;
					comp_set.push_back(comp_node);
				}
				num_of_components++;
			}
						
			assert(num_of_components > 1);
			if (comp_set.size() < 2) {
				cout << "!!!!!!!!!!!!!!!!!!! Compound with one node !!!!!!!!!!!!\n";
			}

			sort(comp_set.begin(), comp_set.end(), compObject);

			string comp_display_name = "c";
            string comp_name = "c";
			for(vector<Node*>::iterator comp_set_it = comp_set.begin(); comp_set_it != comp_set.end(); comp_set_it++) {
				comp_display_name = comp_display_name + "_" + (*comp_set_it)->rep_display_name;
				comp_name = comp_name + "_" + (*comp_set_it)->rep_id_name;
			}
			
			name = comp_name;
			node_display_name = comp_display_name;
			
		}
		

        string name_delim = " ";
		string display_delim = ", ";
	
//		if (type == "gene" || type == "compound") {
//			display_delim = ", ";
//			name_delim = " ";
//		}
//		else if (type == "group") {
//			display_delim = " ";
//			name_delim = " ";
//		}
	

		list<string> *pList_of_names = gm->get_list_of_names_from_string(name, name_delim);
		list<string> list_of_names = *pList_of_names;

		//***comment out starts
		/*ofstream fout;
		fout.open("all_hsaids_164_pathways", std::ofstream::out | std::ofstream::app);

		for(list<string>::iterator itr = list_of_names.begin(); itr != list_of_names.end(); itr++){
			fout << *itr << endl;
		}

		fout.close();*/
		//***comment out ends
			
		int found_earlier_rep_id = 0;
		string rep_id_name;
		string smallest_rep_id_name = list_of_names.front();
		
		for (list<string>::iterator list_it = list_of_names.begin(); list_it != list_of_names.end(); list_it++) {
			rep_id_name = gm->get_and_update_rep_id_name(*list_it);
			
			if (!gm->invalid_string(rep_id_name)) {
				found_earlier_rep_id = 1;
				if (rep_id_name < smallest_rep_id_name) {
					smallest_rep_id_name = rep_id_name;
				}
			}
			else {
				if (*list_it < smallest_rep_id_name) {
					smallest_rep_id_name = *list_it;
				}
			}
		}
		
		
		for (list<string>::iterator list_it = list_of_names.begin(); list_it != list_of_names.end(); list_it++) {
			
			rep_id_name = gm->get_and_update_rep_id_name(*list_it);
			if (rep_id_name != smallest_rep_id_name) {
				gm->add_to_id_name_map(*list_it, smallest_rep_id_name);
				
			}
			rep_id_name = smallest_rep_id_name; // added on Oct 7
		}
		
		delete pList_of_names;

		list<string> * pList_of_display_names = gm->get_list_of_names_from_string(node_display_name, display_delim);
		list<string> list_of_display_names = *pList_of_display_names;
		
		int found_earlier_rep_display = 0;
		string rep_display_name;
		string smallest_rep_display_name = list_of_display_names.front();
		
		/* temporarily commented on 16 Nov 2013 */
//		for (list<string>::iterator list_it = list_of_display_names.begin(); list_it != list_of_display_names.end(); list_it++) {
//			
//		        rep_display_name = gm->get_and_update_rep_display_name(*list_it);
//		
//			if (!gm->invalid_string(rep_display_name)) {
//				//found_earlier_rep_display = 1; // commented on 16 Nov 2013 -- seems like not needed
//				if (rep_display_name < smallest_rep_display_name) {
//					smallest_rep_display_name = rep_display_name;
//				}
//			}
//			else {
//				if (*list_it < smallest_rep_display_name) {
//					smallest_rep_display_name = *list_it;
//				}
//			}
//			
//			
//		}
		/* temporarily commented on 16 Nov 2013 */
		
		for (list<string>::iterator list_it = list_of_display_names.begin(); list_it != list_of_display_names.end(); list_it++) {
			rep_display_name = gm->get_and_update_rep_display_name(*list_it);
			if (rep_display_name != smallest_rep_display_name) {
				gm->add_to_display_name_map(*list_it, smallest_rep_display_name);
			}
			rep_display_name = smallest_rep_display_name; // added on 7 Oct 2013
		}
              
		delete pList_of_display_names;
		
		Node * new_node;

		/* Following line changed on 16 Dec 2013 */
		//// if ( new_graph.find(name) == new_graph.end() ) {
		if ( new_graph.find(rep_id_name) == new_graph.end() ) {
			
			new_node = gm->create_node(rep_id_name, rep_display_name, comp_set, type, link);///// Changed name to rep_id_name
			//cout << "New node " << new_node->rep_id_name << " has id " << new_node->node_id << endl; 
		}
		else {
			/* Following line changed on 16 Dec 2013 */
			//// new_node = (new_graph.find(name))->second;
			new_node = (new_graph.find(rep_id_name))->second;
			//cout << "Existing node " << new_node->rep_id_name << " has id " << new_node->node_id << endl; 
		}
		
		
		////new_graph.insert(pair<string ,Node *>(name, new_node));
		new_graph.insert(pair<string ,Node *>(rep_id_name, new_node)); //// Changed name to rep_id_name

		if(rep_id_name != new_node->rep_id_name){
			cout << rep_id_name << " mismatches " << new_node->rep_id_name << endl;
		}

        gm->add_to_local_to_hsa_node_id_map(new_node->node_id, new_node->rep_id_name);
		node_id_map.insert(pair<int,Node *>(num, new_node));
	
	}
	
	
	// For edges in XML file
		
	xmlpp::Node::NodeList edges_list = pNode->get_children("relation");
	
	for(xmlpp::Node::NodeList::iterator iter = edges_list.begin(); iter != edges_list.end(); ++iter) {
		// Get attributes of edge. eg., source node, target node, type (PPrel, ..), etc.
		int source, target;
		string type;
		list <string> subtype;
	
		const xmlpp::Element* edgeElement = dynamic_cast<const xmlpp::Element*>(*iter);
		const xmlpp::Element::AttributeList& edge_attr_list = edgeElement->get_attributes();
		source = atoi(edgeElement->get_attribute_value("entry1").c_str());
		target = atoi(edgeElement->get_attribute_value("entry2").c_str());
		type = edgeElement->get_attribute_value("type");
					
		// Get source and target nodes of edge
		
		//Node * source_node = gm->get_node_with_num(new_graph, source);
		Node * source_node = (node_id_map.find(source))->second;
		//cout << "source node_id: " << source_node->rep_display_name << " and ";

		//Node * target_node = gm->get_node_with_num(new_graph, target);
		Node * target_node = (node_id_map.find(target))->second;		
		
		
		xmlpp::Node::NodeList edge_subtype_list = edgeElement->get_children("subtype");
		for(xmlpp::Node::NodeList::iterator edge_subtype_iter = edge_subtype_list.begin(); edge_subtype_iter != edge_subtype_list.end(); ++edge_subtype_iter) {
			// Get attributes of subtype. eg., activation, inhibition, etc.
			const xmlpp::Element* edgeSubtypeElement = dynamic_cast<const xmlpp::Element*>(*edge_subtype_iter);
			subtype.push_back(edgeSubtypeElement->get_attribute_value("name"));
		}
				
		Edge * new_edge;
		
		string subtype_str;
		list<string>::iterator it = subtype.begin();
		while (it!= subtype.end()) {
			subtype_str= subtype_str + "_" + (*it);
			it++;
		}
		

                
		//string edge_id = gm->get_and_update_rep_id_name(source_node->rep_id_name) + "_" + gm->get_and_update_rep_id_name(target_node->rep_id_name) + subtype_str;
		////bvatom to_check_edge_id = gm->create_edge_id(source_node, target_node, type, subtype, p_name);
		bvatom to_check_edge_id = gm->create_edge_id(source_node, target_node, type, subtype, pathways);
		//set<string> edge_set = gm->get_all_edges_from_node_map(new_graph);
		
		
		//if (edge_set.find(edge_id) == edge_set.end()) {
		
		if (edge_map.find(to_check_edge_id) == edge_map.end()) {
			////new_edge = gm->create_edge(source_node, target_node, type, subtype, p_name);
                        new_edge = gm->create_edge(source_node, target_node, type, subtype, pathways);
			//edge_map.insert(pair<unsigned int, char>(new_edge->edge_id, 'a'));
			edge_map.insert(new_edge->edge_id);
			
		}
		
		
		
	}
		
//	string graph_genesis_operation = "Read graph from file " + inputfilename;
//	list<int> operand_graphs;
//	string other_params = "";
//	
//	Graph * final_graph = gm->register_new_graph(new_graph, graph_genesis_operation, operand_graphs, other_params);
	
	//bool success = gm->add_new_graph(new_graph);
	node_id_map.empty();
	return new_graph;

		
}

//////
////// SBMLParser::SBMLParser() {
////// }
//////
////// SBMLParser::~SBMLParser() {
////// }

/*
void SBMLParser::process_edge_sbml(GraphNew* graph_ptr, int source_node_id, int target_node_id, string reaction_desc, GraphManagerNew* graph_man, string pathway){
    
    cout << "Edge" << endl;
    bool is_undirected = false;
    string type = "p";
        vector<string> subtype;
        std::locale loc;
        for (std::string::size_type i=0; i<reaction_desc.length(); ++i)
               reaction_desc[i] = std::toupper(reaction_desc[i],loc);
        //cout << reaction_desc;
        std::size_t found;
        
        found = reaction_desc.find("CLEAVAGE");
        if(found!=std::string::npos){
            subtype.push_back("Cleavage");
        }
        
        found = reaction_desc.find("PHOSPHORYLAT");
        if(found!=std::string::npos){
            subtype.push_back("Phosphorylation");
        }
        
        found = reaction_desc.find("UBIQUITINAT");
        if(found!=std::string::npos){
            subtype.push_back("Ubiquitination");
        }
        
        found = reaction_desc.find("DEGRAD");
        if(found!=std::string::npos){
            subtype.push_back("Degradation");
        }
        
        found = reaction_desc.find("ACTIVAT");
        if(found!=std::string::npos){
            subtype.push_back("Activation");
        }
        int eid = graph_ptr->check_if_edge_already_created(source_node_id, target_node_id, type, subtype);

	if( eid == -1){
		//eid = ++GraphManagerNew::edge_id_count;
                eid = graph_ptr->create_new_edge(); // creates a new edge -- added by sukanya
                graph_ptr->add_edge_id(eid); // added by sukanya
		graph_man->add_edge_id_graph_id(eid, graph_ptr->get_graph_id());
		graph_ptr->add_edge_type(eid, type);
		graph_ptr->add_edge_to_outlist_of_node(source_node_id, eid);
		graph_ptr->add_edge_to_inlist_of_node(target_node_id, eid);
		graph_ptr->add_source_node(eid, source_node_id);
		graph_ptr->add_target_node(eid, target_node_id);
		for(vector<string>::iterator subt_itr = subtype.begin(); subt_itr != subtype.end(); subt_itr++){
			graph_ptr->add_subtype_for_edge(eid, *subt_itr);
		}
                
                graph_ptr->add_pathway_for_edge(eid, pathway);
		
	}

	//the undirected case of assoc and dissoc needs edges in both directions in our representation
	if(is_undirected){
		int eid = graph_ptr->check_if_edge_already_created(target_node_id, source_node_id, type, subtype);

		if( eid == -1){
			//eid = ++GraphManagerNew::edge_id_count;
                        eid = graph_ptr->create_new_edge(); // creates a new edge -- added by sukanya
			graph_ptr->add_edge_id(eid);
			graph_man->add_edge_id_graph_id(eid, graph_ptr->get_graph_id());
			graph_ptr->add_edge_type(eid, type);
			graph_ptr->add_edge_to_outlist_of_node(target_node_id, eid);
			graph_ptr->add_edge_to_inlist_of_node(source_node_id, eid);
			graph_ptr->add_source_node(eid, target_node_id);
			graph_ptr->add_target_node(eid, source_node_id);
			for(vector<string>::iterator subt_itr = subtype.begin(); subt_itr != subtype.end(); subt_itr++){
				graph_ptr->add_subtype_for_edge(eid, *subt_itr);
			}

			graph_ptr->add_pathway_for_edge(eid, pathway);
			
		}
	}
        

}*/

