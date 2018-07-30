#include"Interpreter.h"

/* 用来返回pos位置之后下一个word或者符号 */
string Interpreter::getNextString(string s, int *pos){
    string word,tempstring;
    int beginPos, endPos;
    
    /* 初始位置移到没有空格为止 */
    while ((s[*pos]==' ' || s[*pos]=='\n' || s[*pos]=='\t' || s[*pos]=='\r' || s[*pos]==0) && s[*pos]!='\0'){
        (*pos)++;
    }
    beginPos=*pos;
	
	tempstring = s.substr(beginPos,2);
	if(tempstring=="‘"||tempstring=="’"){      // 如果使用了中文输入法的单引号 
		word="ERROR_IME";
		return word;
	}
	
    /* 用来返回 左括号 右括号 逗号 单引号 分号*/
    if(s[*pos]=='(' || s[*pos]==',' || s[*pos]==')' || s[*pos]==';' || s[*pos]=='\'' || s[*pos]=='*'){
        endPos=++(*pos);
        word = s.substr(beginPos,endPos-beginPos);
        return word;
    }

    /* 用来返回除以上符号之外的string */
    else{
        while(s[*pos]!=' ' && s[*pos]!='\n' && s[*pos]!='\t' && s[*pos]!='\r' && s[*pos]!='(' && s[*pos]!=',' && s[*pos]!=')' && s[*pos]!=0 && s[*pos]!='\'' && s[*pos]!=';'  && s[*pos]!='\0' && s[*pos]!='*'){          
			(*pos)++; 
        }
        endPos=*pos;
        if(endPos==beginPos){
            word="";
            return word;
        }
        word=s.substr(beginPos,endPos-beginPos);
        return word;
    }
}

/* 判断是否是符号 */ 
bool Interpreter::isWord(string s){
    if(s[0]=='\'' || s[0]==';' || s[0]==',' || s[0]=='(' || s[0]==')'){
        return false;
    }
    return true;
}

int Interpreter::mainInterpreter(string s){
    string word;
    int pos=0;
    word=getNextString(s,&pos);

    /* create相关语义解释 */
    /* create *****************************************************************************************/
    if(word=="create"){
    	
        int cntbreak=0;                     //循环达到一定次数时强制跳出
        word=getNextString(s,&pos);
        /* 建表 */
        if(word=="table"){
            string primarykey="";
            string tablename="";
            word=getNextString(s,&pos);
            if(!isWord(word)){
                cout<<"Syntax Error: give me a table name please"<<endl;
                return ERROR_SYNTAX;
            }
            tablename=word;

            word=getNextString(s,&pos);
            if(word!="("){
                cout<<"Sytax Error: expect a \'(\' before \""<< word  << "\"" << endl;
                return ERROR_SYNTAX;
            }
            /* 属性获取 */
            word=getNextString(s,&pos);
            vector<Attribute> attrList;
            
            while(!word.empty() && word!="primary" && word!=")"){
            	
                cntbreak++;
                string attrName = word;
                
                int type;
                bool unique=false;
                word=getNextString(s,&pos);
                if(word=="int"){
                    type=TYPE_INT;
                }else if(word=="float"){
                    type=TYPE_FLOAT;
                }else if(word=="char"){
                    word=getNextString(s,&pos);
                    if(word!="("){
                        cout<<"Syntax Error: expect a \'(\' after char"<<endl;
                        return ERROR_SYNTAX;
                    }
                    word=getNextString(s,&pos);
                    stringstream ss;                      //使用字符串流获得type  待定
                    ss << word;
                    if(!(ss>>type)){
                        ss<<"";
                        cout<<"Syntax Error：illegal number in char()"<<endl;
                        return ERROR_SYNTAX;
                    }
                    ss<<"";
                    word=getNextString(s,&pos);
                    
                    if(word!=")"){
                        cout<<"Sytax Error: expect a \')\' after char( "<<endl;
                        return ERROR_SYNTAX;
                    }
                }else{
                    cout<<"Sytax Error: expect a tpye after each attribute"<<endl;
                    return ERROR_SYNTAX;
                }
                word=getNextString(s,&pos);
                if(word=="unique"){                                   //属性后面直接加上的 unique和primary 
                	unique=true;
                	word=getNextString(s,&pos);
				} 
                if(word=="primary"){
                	word=getNextString(s,&pos);
                	if(word!="key"){
                		cout<<"Syntax Error: expect a \"key\" after \"primary\""<<endl;
						return ERROR_SYNTAX;
					}
					unique=true;
					primarykey=attrName;
					word=getNextString(s,&pos);
				}
                Attribute attr(attrName,type,unique);
                attrList.push_back(attr);
                if(word!="," && word!=")" && !word.empty()){  //如果下一个直接是下一个属性单词 
                    cout<<"Syntax Error: expect a\",\" between your attribute" <<endl;
                    return ERROR_SYNTAX;
                }
				else if(word==")"){                     //检查如果下一个是括号的话 代表属性结束 
                    break;
                }
				else if(cntbreak>=100){                 // 做多100个属性
                    cout<<"Sytax Error: expect a\")\" at the end of create table"<<endl;
                    cout<<" OR you may not create a table with the amount of attributes lager than 100"<<endl;
                    return ERROR_SYNTAX;
                }
                word=getNextString(s,&pos);             //如果是正常逗号的话，跳过逗号读取下一个单词 
                if(!isWord(word)){                      //如果逗号后面不是正常的单词的话 
                	cout<<"Syntax Error: unexpected char in the create table"<<endl;
                	return ERROR_SYNTAX;
				}
            }//end of while to fatch attribute

//	
                
            if(word=="primary"){
                word=getNextString(s,&pos);
                if(word!="key"){
                    cout<<"Syntax Error: expect \"key\" after \"primary\""<<endl;
                    return ERROR_SYNTAX;
                }
                word=getNextString(s,&pos);
                if(word!="("){
                    cout<<"Syntax Error: expect a \"(\" after \"primary key\""<<endl;
                    return ERROR_SYNTAX;
				}            
                word=getNextString(s,&pos);
                primarykey=word;
                int i;
                for(i=0;i<attrList.size();i++){
                    if(primarykey==attrList[i].name){
                        attrList[i].ifUnique=true;               //将主键对应属性置为unique
                        break;
                    }
                }
                if(i==attrList.size()){
                    cout<<"Syntax Error: primary key dose not exist"<<endl;
                    return ERROR_SYNTAX;
                }
                word=getNextString(s,&pos);
                if(word!=")"){
                    cout<<"Syntax Eroor: expect a \")\" after \"primary key (\""<<endl;
                    return ERROR_SYNTAX;
                }
            }// end of primary

            if(word!=")"){
                cout<<"Syntax Error: expect a \")\" at the end of create table"<<endl;
                return ERROR_SYNTAX;
            }
            api->CreateTable(tablename,&attrList,primarykey);
            return TRUE_SYNTAX;
        }//end of table
        
// 以上debug完成！ 




        else if(word=="index"){
            string indexname="";
            string tablename="";
            string attrname="";
            word=getNextString(s,&pos);
            if(!isWord(word) || word=="on"){
                cout<<"Syntax Error: expect a index name"<<endl;
                return ERROR_SYNTAX;
            }
            indexname=word;
            word=getNextString(s,&pos);
            if(word!="on"){
                cout<<"Syntax Error: expect \"on\" in create index"<<endl;
                return ERROR_SYNTAX;
            }
            word=getNextString(s,&pos);
            if(!isWord(word)){
                cout<<"Syntax Eroor: expect a table name in create index"<<endl;
                return ERROR_SYNTAX;
            }
            tablename=word;
            word=getNextString(s,&pos);
            if(word!="("){
                cout<<"Syntax Error: expect a \"(\" after the table name"<<endl;
                return ERROR_SYNTAX;
            }
            word=getNextString(s,&pos);
            if(!isWord(word) || word.empty()){
                cout<<"Syntax Error: expect attribute name in \"tablename()\""<<endl;
                return ERROR_SYNTAX;
            }
            attrname=word;
            word=getNextString(s,&pos);
            if(word!=")"){
                cout<<"Syntax Error: expect \")\" after the \"tablename(\""<<endl;
                return ERROR_SYNTAX;
            }
            api->CreateIndex(indexname,tablename,attrname);
            return TRUE_SYNTAX;
        }//end of index
        else{
            cout<<"Syntax Error: sorry you can not create such thing "<<word<<endl;
            return ERROR_SYNTAX;
        }
    }/* end of create *********************************************************************************/
    
//以上debug完成！ 

     /* let's start drop~ *****************************************************************************/
    else if(word=="drop"){
        string tablename;
        string indexname;
        word=getNextString(s,&pos);
        if(word=="table"){
            word=getNextString(s,&pos);
            if(!isWord(word)){
                cout<<"Syntax Error: expect a table name in drop table"<<endl;
                return ERROR_SYNTAX;
            }
            tablename=word;
            api->DropTable(tablename);
            return TRUE_SYNTAX;
        }// end if drop table
        else if(word=="index"){
            word=getNextString(s,&pos);
            if(!isWord(word)){
                cout<<"Syntax Error: expect a index name in drop index"<<endl;
                return ERROR_SYNTAX;
            }
            indexname=word;
            api->DropIndex(indexname);
            return TRUE_SYNTAX;
        }// end of drop index
        else{
            cout <<"Syntax Error: sorry you can not drop "<<word<<endl;
        }
    }/* end of drop ************************************************************************************/

//以上debug完成！ 

     /* let's start insert~*****************************************************************************/
    else if(word=="insert"){
        int cntbreak=0;                              //用来强制退出
        string tablename="";
        vector<string>valueList;
        word=getNextString(s,&pos);
        if(word!="into"){
            cout<<"Syntax Error: expect \"into\" after insert"<<endl;
            return ERROR_SYNTAX;
        }
        word=getNextString(s,&pos);
        if( word.empty() || word=="values" || !isWord(word)){
            cout<<"Syntax Error: expect a table name in insert"<<endl;
            return ERROR_SYNTAX;
        }
        tablename=word;
        word=getNextString(s,&pos);
        if(word!="values"){
            cout<<"Syntax Error: expect \"values\" after tablename"<<endl;
            return ERROR_SYNTAX;
        }
        word=getNextString(s,&pos);
        if(word!="("){
            cout<<"Syntax Error: expect \"(\" after \"into\""<<endl;
            return ERROR_SYNTAX;
        }
        word=getNextString(s,&pos);
        while(word!=")"){                                // fetch values
            cntbreak++;
            if(word=="ERROR_IME"){
            	cout<<"Syntax Error: you should not use Chinese input method to enter \"‘’\""<<endl;
				return ERROR_SYNTAX; 
			} 
            if(word=="\'"){                              // 我这个interpreter对value的单引号要求不严格 
                word=getNextString(s,&pos);
            }
            valueList.push_back(word);
            word=getNextString(s,&pos);
            if(word=="ERROR_IME"){
            	cout<<"Syntax Error: you should not use Chinese input method to enter \"‘’\""<<endl;
				return ERROR_SYNTAX; 
			} 
            if(word=="\'"){
                word=getNextString(s,&pos);
            }
            if(word==","){
                word=getNextString(s,&pos);
                if(word==")"){
                    cout<<"Syntax Error: extra \",\" in insert values"<<endl;
                    return ERROR_SYNTAX;
                }
            }
            else if(word!="," && word!=")" && !word.empty()){
                cout<<"Syntax Error: expect \",\" between two values"<<endl;
                return ERROR_SYNTAX;
            }
            if(cntbreak>=100){                          // 防止用户没加 ） 而死循环
                cout<<"Syntax Error: CNTBREAK! do you forget to use \")\" in the end?"<<endl;
                return ERROR_SYNTAX;
            }
        }// end of while to get values

        api->InsertRecord(tablename,&valueList);
        return TRUE_SYNTAX;
    }/* end of insert****************************************************************************/

//以上debug成功！ 

     /* let's start delete***********************************************************************/

    else if(word=="delete"){
        int cntbreak=0;
        string tablename="";
        word=getNextString(s,&pos);
        if(word!="from"){
            cout << "Syntax Error: expect \"from\" after \"delete\""<<endl;
            return ERROR_SYNTAX;
        }
        word=getNextString(s,&pos);
        if(word.empty()||word=="where"){
            cout<< "Syntax Error: expect table name when you delete a record"<<endl;
            return ERROR_SYNTAX;
        }
        tablename=word;
        word=getNextString(s,&pos);
        if(word==";"||word.empty()){                   // 如果后面没有东西了，就代表无条件delete
            api->DeleteRecord(tablename);
            return TRUE_SYNTAX;
        }
        if(word!="where"){
            cout<<"Syntax Error: expect \"where\" before conditions"<<endl;
            return ERROR_SYNTAX;
        }
        string attrname="";
        string value="";
        vector<Condition> conditionList;
        int op=0;
        word=getNextString(s,&pos);
        while(true){
            cntbreak++;
            if(word.empty()||word==";"){
                cout<<"Syntax Error: you did not enter any conditions"<<endl;
                return ERROR_SYNTAX;
            }
            attrname=word;
            word=getNextString(s,&pos);
            if(word=="=")op=OP_EQUAL;
            else if(word=="<")op=OP_LESS;
            else if(word==">")op=OP_GREATER;
            else if(word==">=")op=OP_GREATER_EQUAL;
            else if(word=="<=")op=OP_LESS_EQUAL;
            else if(word=="<>")op=OP_NOT_EQUAL;
            else{
                cout<<"Syntax Error: unkonwn operator Note:you must enter your operater with two blanks in front of and after the operater"<<endl;
                return ERROR_SYNTAX;
            }
            word=getNextString(s,&pos);
            if(word=="ERROR_IME"){
            	cout<<"Syntax Error: you should not use Chinese input method to enter \"‘’\""<<endl;
				return ERROR_SYNTAX; 
			} 
            if(word=="\'"){
                word=getNextString(s,&pos);
            }
            if(word.empty() || word=="and" || !isWord(word)){
                cout<<"Syntax Error: expect a value after \"where\""<<endl;
                return ERROR_SYNTAX;
            }
            value=word;
            word=getNextString(s,&pos);
            if(word=="ERROR_IME"){
            	cout<<"Syntax Error: you should not use Chinese input method to enter \"‘’\""<<endl;
				return ERROR_SYNTAX; 
			} 
            if(word=="\'"){
                word=getNextString(s,&pos);
            }
            Condition cdt(attrname,op,value);
            conditionList.push_back(cdt);
            if(word==";" || word.empty()){
                break;
            }else if(cntbreak>100){
                cout<<"Syntax Error: CNTBREAK can not hold too many conditions in delete where"<<endl;
                return ERROR_SYNTAX;
            }
            else if (word!="and"){
                cout<<"Syntax Error: expect \"and\" or \";\""<<endl;
                return ERROR_SYNTAX;
            }
            word=getNextString(s,&pos);
        }
        api->DeleteRecord(tablename,&conditionList);
        return TRUE_SYNTAX;
    }/* end of delete*************************************************************************/

//以上debug完成！ 

     /* let's start select********************************************************************/
    else if(word=="select"){
        int cntbreak=0;
        vector<string> attrSelect;
        string tablename="";
        word=getNextString(s,&pos);
        
        if(word!="*"){   // 我们实现了select *以外的功能 
			if(word=="from" || word.empty()){
        		cout<<"Syntax Error: after select, you need to enter * or attributes"<<endl;
        		return ERROR_SYNTAX;
			}                          
            while(word!="from"){
                cntbreak++;
                attrSelect.push_back(word);
                word=getNextString(s,&pos);
                if(word==","){
                    word=getNextString(s,&pos);
                }
                if(cntbreak>100){
                    cout<<"Syntax Error: CNTBREAK can not hold too many attributes in select"<<endl;
                    return ERROR_SYNTAX;
                }
            }// end of while
        }// end of if *
        else if(word=="*"){
            word=getNextString(s,&pos);
        }
        if(word!="from"){
            cout<<"Syntax Error: expect \"from\" after select"<<endl;
            return ERROR_SYNTAX;
        }
        word=getNextString(s,&pos);
        if(word=="where" || !isWord(word) || word.empty()){
            cout<<"Syntax Error: expect tablename after \"from\""<<endl;
            return ERROR_SYNTAX;
        }
        tablename=word;
        word=getNextString(s,&pos);
        if(word.empty() || word==";"){        //如果没有where条件
            if(attrSelect.size()==0){
                api->SelectRecord(tablename);
            }else{
                api->SelectRecord(tablename,&attrSelect);
            }
            return TRUE_SYNTAX;
        }
        if(word!="where"){
            cout<<"Syntax Error: expect \"where\" in select"<<endl;
            return ERROR_SYNTAX;
        }
        cntbreak=0;                          // cntbreak置为0
        string attrname="";
        string value="";
        vector<Condition> conditionList;
        int op=0;
        word=getNextString(s,&pos);
        while(true){
            cntbreak++;
            if(word.empty()||word==";"){
                cout<<"Syntax Error: u must enter your conditions when you have entered \"where\" or \"and\""<<endl;
                return ERROR_SYNTAX;
            }
            attrname=word;
            word=getNextString(s,&pos);
            if(word=="=")op=OP_EQUAL;
            else if(word=="<")op=OP_LESS;
            else if(word==">")op=OP_GREATER;
            else if(word==">=")op=OP_GREATER_EQUAL;
            else if(word=="<=")op=OP_LESS_EQUAL;
            else if(word=="<>")op=OP_NOT_EQUAL;
            else{
                cout<<"Syntax Error: unkonwn operator Note:you must enter your operater with two blanks in front of and after the operater"<<endl;
                return ERROR_SYNTAX;
            }
            word=getNextString(s,&pos);
            if(word=="ERROR_IME"){
            	cout<<"Syntax Error: you should not use Chinese input method to enter \"‘’\""<<endl;
				return ERROR_SYNTAX; 
			} 
            if(word=="\'"){
                word=getNextString(s,&pos);
            }
            if(word.empty()){
                cout<<"Syntax Error: expect a value after \"where\""<<endl;
                return ERROR_SYNTAX;
            }
            value=word;
            word=getNextString(s,&pos);
            if(word=="ERROR_IME"){
            	cout<<"Syntax Error: you should not use Chinese input method to enter \"‘’\""<<endl;
				return ERROR_SYNTAX; 
			} 
            if(word=="\'"){
                word=getNextString(s,&pos);
            }
            Condition cdt(attrname,op,value);
            conditionList.push_back(cdt);
            
            if(word==";" || word.empty()){
                break;
            }else if(cntbreak>100){
                cout<<"Syntax Error: CNTBREAK can not hold too many conditions in select where"<<endl;
                return ERROR_SYNTAX;
            }
            else if (word!="and"){
                cout<<"Syntax Error: expect \"and\" or \";\""<<endl;
                return ERROR_SYNTAX;
            }
            word=getNextString(s,&pos);
        }// end of where
        if(attrSelect.size()==0){
            api->SelectRecord(tablename,NULL,&conditionList);
        }else{
            api->SelectRecord(tablename,&attrSelect,&conditionList);
        }
        return TRUE_SYNTAX;
    }
    else if(word=="quit"){
        return QUIT;
    }else if(word=="execfile"){
        FileName=getNextString(s,&pos);
        cout << "Opening" <<FileName <<"..."<<endl;
        return FILEREAD;
    }else{
    	if(!word.empty()){
      	    cout<<"Command: " << word << " ERROR" << endl;
            return ERROR_SYNTAX;
        }else{
        	return TRUE_SYNTAX;
		}
    }
}
