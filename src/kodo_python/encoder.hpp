// Copyright Steinwurf ApS 2015.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <Python.h>
#include <bytesobject.h>

#include <boost/python/args.hpp>

#include <kodo/has_trace.hpp>
#include <kodo/has_set_systematic_off.hpp>
#include <kodo/is_systematic_on.hpp>
#include <kodo/set_systematic_off.hpp>
#include <kodo/set_systematic_on.hpp>
#include <kodo/write_feedback.hpp>
#include <kodo/rlnc/sparse_full_vector_encoder.hpp>

#include <string>
#include <vector>

#include "coder.hpp"
#include "resolve_field_name.hpp"

namespace kodo_python
{
    template<class Encoder>
    bool is_systematic_on(const Encoder& encoder)
    {
        return kodo::is_systematic_on(encoder);
    }

    template<class Encoder>
    void set_systematic_on(Encoder& encoder)
    {
        kodo::set_systematic_on(encoder);
    }

    template<class Encoder>
    void set_systematic_off(Encoder& encoder)
    {
        kodo::set_systematic_off(encoder);
    }

    template<class Encoder>
    void set_symbols(Encoder& encoder, const std::string& data)
    {
        auto storage = sak::const_storage(
            (uint8_t*)data.c_str(), data.length());
        encoder.set_symbols(storage);
    }

    template<class Encoder>
    void set_symbol(Encoder& encoder, uint32_t index, const std::string& data)
    {
        auto storage = sak::const_storage(
            (uint8_t*)data.c_str(), data.length());
        encoder.set_symbol(index, storage);
    }

    template<class Encoder>
    PyObject* encoder_write_payload(Encoder& encoder)
    {
        std::vector<uint8_t> payload(encoder.payload_size());
        uint32_t length = encoder.write_payload(payload.data());
        #if PY_MAJOR_VERSION >= 3
        return PyBytes_FromStringAndSize((char*)payload.data(), length);
        #else
        return PyString_FromStringAndSize((char*)payload.data(), length);
        #endif
    }

    template<class Encoder>
    void read_feedback(Encoder& encoder, const std::string& feedback)
    {
        std::vector<uint8_t> _feedback(feedback.length());
        std::copy(
            feedback.c_str(),
            feedback.c_str() + feedback.length(),
            _feedback.data());
        encoder.read_feedback(_feedback.data());
    }

    template<bool IS_SYSTEMATIC_ENCODER, class Type>
    struct systematic_encoder_methods
    {
        template<class EncoderClass>
        systematic_encoder_methods(EncoderClass& encoder_class)
        {
            (void) encoder_class;
        }
    };

    template<class Type>
    struct systematic_encoder_methods<true, Type>
    {
        template<class EncoderClass>
        systematic_encoder_methods(EncoderClass& encoder_class)
        {
            encoder_class
            .def("is_systematic_on", &is_systematic_on<Type>,
                "Check if the encoder is in systematic mode.\n\n"
                "\t:returns: True if the encoder is in systematic mode.\n"
            )
            .def("set_systematic_on", &set_systematic_on<Type>,
                "Set the encoder in systematic mode.\n"
            )
            .def("set_systematic_off", &set_systematic_off<Type>,
                "Turn off systematic mode.\n");
        }
    };

    template<template<class, class> class Coder, class Type>
    struct extra_encoder_methods
    {
        template<class EncoderClass>
        extra_encoder_methods(EncoderClass& encoder_class)
        {
            (void) encoder_class;
        }
    };

    template<class Type>
    struct extra_encoder_methods<kodo::rlnc::sliding_window_encoder, Type>
    {
        template<class EncoderClass>
        extra_encoder_methods(EncoderClass& encoder_class)
        {
            encoder_class
            .def("feedback_size", &Type::feedback_size,
                "Return the required feedback buffer size in bytes.\n\n"
                "\t:returns: The required feedback buffer size in bytes.\n"
            )
            .def("read_feedback", &read_feedback<Type>,
                "Return the feedback information.\n\n"
                "\t:returns: The feedback information.\n");
        }
    };


    template<class Type>
    struct extra_encoder_methods<kodo::rlnc::perpetual_encoder, Type>
    {
        template<class EncoderClass>
        extra_encoder_methods(EncoderClass& encoder_class)
        {
            using boost::python::arg;

            encoder_class
            .def("pseudo_systematic", &Type::pseudo_systematic,
                "Get the pseudo-systematic property of the generator.\n\n"
                "\t:returns: The current setting for pseudo-systematic.\n"
            )
            .def("set_pseudo_systematic", &Type::set_pseudo_systematic,
                arg("pseudo_systematic"),
                "Set the pseudo-systematic property of the generator.\n\n"
                "\t:param pseudo_systematic: The new setting for "
                "pseudo-systematic\n"
            )
            .def("pre_charging", &Type::pre_charging,
                "Get the pre-charging property of the generator.\n\n"
                "\t:returns: The current setting for pseudo-systematic.\n"
            )
            .def("set_pre_charging", &Type::set_pre_charging,
                arg("pre_charging"),
                "Set the pre-charging property of the generator.\n\n"
                "\t:param pre_charging: The current setting for pre-charging.\n"
            )
            .def("width", &Type::width,
                "Get the width.\n\n"
                "\t:returns: The width used by the generator.\n"
            )
            .def("set_width", &Type::set_width,
                arg("width"),
                "Set the number of non-zero coefficients after the pivot. "
                "Width ratio is recalculated from this value.\n\n"
                "\t:param width: The width.\n"
            )
            .def("width_ratio", &Type::width_ratio,
                "Get the ratio that is used to calculate the width.\n\n"
                "\t:returns: The width ratio of the generator.\n"
            )
            .def("set_width_ratio", &Type::set_width_ratio,
                arg("width_ratio"),
                "Set the ratio that is used to calculate the number of "
                "non-zero coefficients after the pivot (i.e. the width).\n\n"
                "\t:param width_ratio: The width ratio.\n"
            );
        }
    };

    template<class Type>
    struct extra_encoder_methods<kodo::rlnc::sparse_full_vector_encoder, Type>
    {
        template<class EncoderClass>
        extra_encoder_methods(EncoderClass& encoder_class)
        {
            using boost::python::arg;
            encoder_class
            .def("set_density", &Type::set_density, arg("density"),
                "Set the density of the coefficients generated.\n\n"
                "\t:param density: The coefficients density.\n"
            )
            .def("density", &Type::density,
                "Get the density of the coefficients generated.\n\n"
                "\t:returns: The density of the generator.\n"
            )
            .def("set_average_nonzero_symbols",
                &Type::set_average_nonzero_symbols, arg("symbols"),
                "Set the average number of nonzero symbols.\n\n"
                "\t:param symbols: The average number of nonzero symbols.\n"
            );
        }
    };

    template<template<class, class> class Coder, class Field, class TraceTag>
    void encoder(const std::string& stack)
    {
        using boost::python::arg;
        using boost::python::args;
        using encoder_type = Coder<Field, TraceTag>;

        std::string field = resolve_field_name<Field>();
        std::string kind = "Encoder";
        std::string name = stack + kind + field;

        auto encoder_class = coder<Coder, Field, TraceTag>(name)
        .def("write_payload", &encoder_write_payload<encoder_type>,
            "Encode a symbol.\n\n"
            "\t:returns: The encoded symbol.\n"
        )
        .def("set_symbols", &set_symbols<encoder_type>, arg("symbols"),
            "Set the symbols to be encoded.\n\n"
            "\t:param symbols: The symbols to be encoded.\n"
        )
        .def("set_symbol", &set_symbol<encoder_type>, args("index", "symbol"),
            "Set a symbol to be encoded.\n\n"
            "\t:param index: The index of the symbol in the coding block.\n"
            "\t:param symbol: The actual data of that symbol.\n");

        (extra_encoder_methods<Coder, encoder_type>(encoder_class));

        (systematic_encoder_methods<
            kodo::has_set_systematic_off<encoder_type>::value, encoder_type>
            (encoder_class));
    }
}
