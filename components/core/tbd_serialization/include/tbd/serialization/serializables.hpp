#pragma once

#include <tbd/errors.hpp>

#include <fstream>

namespace tbd::serialization {

template<typename SerializableT>
struct _dto_for_type {
    using type = void;
};

template<typename SerializableT>
using dto_for_type = typename _dto_for_type<SerializableT>::type;

template<class SerializableT>
struct SerializableImpl;

template<typename SerializableT>
concept SerializableType = requires (
    SerializableT obj,
    dto_for_type<SerializableT> prop_data,
    const SerializableT const_obj,
    const dto_for_type<SerializableT> const_prop_data,
    std::istream in_stream,
    std::ostream out_stream)
{
    {SerializableImpl<SerializableT>::serialized_size(const_obj)} -> std::same_as<size_t>;
    {SerializableImpl<SerializableT>::type_id()} -> std::same_as<TypeID>;
    {SerializableImpl<SerializableT>::is_directly_serializable()} -> std::same_as<bool>;

    {SerializableImpl<SerializableT>::read_serializable(in_stream, obj)} -> std::same_as<Error>;
    {SerializableImpl<SerializableT>::write_serializable(out_stream, const_obj)} -> std::same_as<Error>;
    {SerializableImpl<SerializableT>::to_dto(const_obj, prop_data)} -> std::same_as<Error>;
    {SerializableImpl<SerializableT>::from_dto(const_prop_data, obj)} -> std::same_as<Error>;
};

template<SerializableType SerializableT>
struct Serializable : SerializableImpl<SerializableT> {};

}
