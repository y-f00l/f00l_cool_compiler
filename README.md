## CS143
Complete a compiler of cool language.
### program assignment
- PA2 lexer  /done
- PA3 parser /done
- PA4 semant analyse /done
- PA5 code generation /done

## note
记录自己的学习
### lexer
 - 词法分析，在这里用的是flex这个工具
 
 - 编程语言是由代码和注释构成的，而代码里一定包含了各种关键字等，这些东西被称为token
 
 - 词法分析器就是将整个源文件拆分成一个一个token，输出到parser中来构建AST(抽象语法树)
 
 - flex是用正则表达式来对源代码进行词法分析的，这些正则表达式就不在这里赘述，可以去看flex的手册
#### 有穷自动机
  - 自动机是实现词法分析的一个重点
  
  - 自动机当接受到特定输入的时候就会从一个状态跳到另一个状态
  
  - 一个词法分析中的例子:
    - 当匹配到//开头的文本的时候，会切到匹配注释的模式，直到匹配到换行。
    - 当匹配到' " '的时候，切换到匹配字符串的模式，直到匹配另一个' " '
#### 需求分析
  - 吞掉注释
  
  - 将源代码拆分成token
    - 数字常量，字符串常量
    - 变量名称
    - 函数名称
    - 系统关键字
    
  - 准确性:在遇到unexpected的情况时，可以处理，不会进行错误的匹配
代码已上传至PA2的文件夹.
### parser
  - 语法分析，将词法分析器的结果构建成AST
  
  - 用了bison这个工具
  
  - bison需要手写文法来构建AST
  
  - 需要规定运算符的优先级来对表达式进行AST的构建
#### cool的文法和运算符优先级
  - 在cool的手册里有写，这里不多赘述
#### bison
  - bison采用了自底向上的规约方法来分析
### semant
  - 语义分析，对AST进行一些检查，对不合规范的代码抛出异常
#### class
  - 类，会对类进行一系列检查，包括属性，方法的检查
#### inheritance graph
  - 继承图，包含了父类和子类之间的继承关系
  
  - 会对继承关系的合法性进行检查，例如
    - 父类是否是可继承的
    - 父类是否定义
#### scope
  - 作用域
  
  - 每个变量都有自己的一个作用域，因为有可能两个变量有相同的名字
  - 在规定了作用域后，变量名发生冲突时就可以按照变量的作用域选择对应变量
  
  - 在规定作用域后，在对变量进行使用的时候就会检查是否在它的域内，如果没有则会抛出错误
  
#### type check
  - 对类型进行检查
    - 对类型定义了一个比较的运算，这是为了方便进行类型转换
    - 查找这个类型是否被定义
    - 变量的值是否符合定义的类型
  
  - 类的属性检查：
    - 是否定义了属性的类型
    - 属性的类型和初始化方法里的类型是否相同或者可以转换
    
  - 类的方法检查:
    - 是否定义了属性的类型
    - 类的返回值类型是否和返回类型相同或者可以转换
  
### code generation
