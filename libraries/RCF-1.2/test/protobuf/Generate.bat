protoc --proto_path=../../include/RCF/protobuf ../../include/RCF/protobuf/RcfMessages.proto --python_out=.
protoc Person.proto --python_out=. --cpp_out=.
