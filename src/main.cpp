#include <iostream>
#include <string>

int main() {
    std::cout << "Hello World" << std::endl;
    std::cout << "Please enter the location of your videos:";
    std::string main_path = "";
    std::cin >> main_path;
    std::cout << "You entered: " << main_path << std::endl;
}