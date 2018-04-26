#include <ki/protocol/dml/MessageManager.h>
#include <ki/protocol/exception.h>
#include <iostream>

using namespace ki::protocol;

int main(int argc, char **argv)
{
	// Get command-line arguments
	if (argc < 3)
	{
		std::cout << "usage: example-dml-module.exe <module_file> <message_name>" << std::endl;
		std::cout << "Prints out information for specified message." << std::endl;
		return 1;
	}

	// Create a manager to load modules into
	auto *message_manager = new dml::MessageManager();
	const dml::MessageModule *message_module;

	// Load the message module file
	const std::string filepath = argv[1];
	try
	{
		message_module = message_manager->load_module(filepath);
	}
	catch (value_error &e)
	{
		std::cout << "Failed to load message module.";
		return 1;
	}

	// Print some information about the module itself
	std::cout << "Service ID: " << (uint16_t)message_module->get_service_id() << std::endl;
	std::cout << "Protocol Type: " << message_module->get_protocol_type() << std::endl;

	// Get the message template from the module we just loaded
	const std::string message_name = argv[2];
	auto *message_template = message_module->get_message_template(message_name);
	if (message_template)
	{
		std::cout << "Message Name: " << message_template->get_name() << std::endl;
		std::cout << "Mesasge Type: " << (uint16_t)message_template->get_type() << std::endl;

		// Print out the fields in the template record
		std::cout << std::endl;
		auto &record = message_template->get_record();
		for (auto it = record.fields_begin();
			it != record.fields_end(); ++it)
		{
			auto *field = *it;
			if (field->is_transferable())
				std::cout << field->get_type_name() << " " << field->get_name() << ";" << std::endl;
		}
	}
	else
	{
		std::cout << "Could not find message with name: " << message_name << std::endl;
		return 1;
	}

	// Exit successfully
	return 0;
}
