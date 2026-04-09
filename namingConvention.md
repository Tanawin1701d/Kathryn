# Kathryn Phase 7.5 naming convention

1. filename and folder name should be the camelCase
2. class name should be the PascalCase
3. class memeber variable should be the _snake_case  (with _ leading)
4. variable should be the snake_case
5. class method should be the snake_case
6. multiple line comment should be /***/
7  no //////// comment
8. comment should have only ///
9. method with ptr return the name must have _ptr suffix such as get_flow_block_ptr
10. pure class/struct without method the class/struct member variable name should be the snake_case (no _ leading)

//// promt for .cpp

1. change all local variable in all method in this file to snakecase but if the variable is reference from external do not change it   make sure all functionality is identical compared with before change
2. do change all single comment syntax to /// instead /*