// Copyright (c) 2014 Christian Kellner <kellner@bio.lmu.de>
// All rights reserved
// License: BSD 3-clause
// clang -O2 -g -o num_inspect -Wall -std=c++11 -lstdc++ -lboost_program_options num_inspect.cc

#include <iostream>
#include <string>
#include <cmath>
#include <limits>
#include <iomanip>
#include <stdexcept>

#include <cstdlib>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

template<typename T>
union FloatInspector {
  T v_float;
  uint8_t v_bits[sizeof(T)];
};

std::string
printbits(uint8_t *data, size_t len)
{
  const size_t bitlen = len*8;
  std::string buf;

  buf.resize(bitlen);
  uint8_t tmp;

  for(int i = 0; i < bitlen; i++) {

    if (i % 8 == 0) {
      tmp = data[i / 8];
    }

    buf[bitlen - (i+1)] = tmp & 1 ? '1' : '0';
    tmp >>= 1;
  }

  return buf;
}

template<typename T>
std::string
printbits(T data)
{
  uint8_t *xdata = reinterpret_cast<uint8_t *>(&data);
  return printbits(xdata, sizeof(data));
}


template<typename T>
struct float_traits {
  static const bool valid = false;
};

template<>
struct float_traits<float> {
  static const size_t bits_s = 1;
  static const size_t bits_e = 8;
  static const size_t bits_p = 23;
};

template<>
struct float_traits<double> {
  static const size_t bits_s = 1;
  static const size_t bits_e = 11;
  static const size_t bits_p = 52;
};


template<typename T, class = typename std::enable_if<std::is_floating_point<T>::value, T>::type>
void show_number(const std::string numstr, T value)
{
  std::string bits = printbits(value);
  std::stringstream valstr;

  valstr  << std::setprecision(100) << value;

  std::cout << std::setprecision(100);
  std::cout << std::endl << std::setw(20) << "Input: " << numstr << std::endl;
  std::cout << std::setw(20) << "Floating Point: " << std::boolalpha << (valstr.str() == numstr);
  if (valstr.str() != numstr) {
    std::cout << std::endl << std::setw(20) << "Internal: " << valstr.str();
  }
  std::cout << std::endl  << std::endl;


  std::cout << std::setw(20) << "bits: ";
  std::cout << "[";
  std::cout << bits.substr(0, float_traits<T>::bits_s) << " ";
  std::cout << bits.substr(float_traits<T>::bits_s, float_traits<T>::bits_e) << " ";
  std::cout << bits.substr(float_traits<T>::bits_s+ float_traits<T>::bits_e, float_traits<T>::bits_p);
  std::cout << "]" << std::endl;
  std::cout << std::setw(20) << "size: " << sizeof(T)*8 << " bits" <<std::endl;
  std::cout << std::setw(20) << "maschine epsilon: " << std::numeric_limits<T>::epsilon() << std::endl;

  std::cout << std::setw(20) << "nextafter: " << std::nextafter(value, 0) << std::endl;

  //
  int exp;
  T significand = frexp (value, &exp);

  std::cout << std::setw(20) << "fp-format: " << significand << " * " << std::numeric_limits<T>::radix << "^" << exp << std::endl;
}


// parsing numbers
template<typename T>
T parse_num_as(std::string numstr)
{
  throw std::runtime_error("Not sure how to parse that number. ;-(");
}

template<>
float parse_num_as<float>(std::string numstr)
{
  return std::strtof(numstr.c_str(), nullptr);
}

template<>
double parse_num_as<double>(std::string numstr)
{
  return std::strtod(numstr.c_str(), nullptr);
}


template<typename T>
void parse_and_show(std::string numstr)
{
  T val = parse_num_as<T>(numstr);
  show_number(numstr, val);
}

template<>
void parse_and_show<std::string>(std::string numstr)
{
  if(numstr.find('.') != std::string::npos) {
    double val = parse_num_as<double>(numstr);
    show_number(numstr, val);
  }
}

struct str2fun {
  std::string name;
  void (*function)(std::string str);
} str2fun_map[] = {
  {"float",  parse_and_show<float>},
  {"double", parse_and_show<double>},
  {"[deduce]", parse_and_show<std::string>}
};

// *****************************************************************************

int main(int argc, char **argv)
{
  po::options_description desc("number inspector");
  po::positional_options_description opts;

  std::string type = "[deduce]";

  desc.add_options()
    ("help,h", "Show this help text")
    ("value", po::value<std::string>()->required(),"The value to inspect")
    ("type", po::value<std::string>(&type), "The type of value [deduce]");;

  opts.add("value", 1);
  opts.add("type", 1);

  po::variables_map vm;


  try {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(opts).run(), vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }

    po::notify(vm);

  } catch (boost::program_options::required_option& e) {
    return -1;
  } catch(boost::program_options::error& e) {
    return -1;
  }

  std::string value = vm["value"].as<std::string>();

  bool found = false;
  for(const auto &parser : str2fun_map) {
    if(parser.name == type) {
      found = true;
      parser.function(value);
    }
  }

  return found ? 0 : -2;
}
