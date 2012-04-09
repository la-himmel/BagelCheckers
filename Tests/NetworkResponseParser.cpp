
#include <FWebJsonJsonParser.h>
#include "Helpers/Typedefs.h"
#include "Helpers/Functions.h"
#include "Network/NetworkJsonKeys.h"
#include "DataManagement/DataItems/Pair.h"
#include "DataManagement/DataItems/FullJobItem.h"
#include "DataManagement/DataItems/UserData.h"
#include "Resources/Strings.h"

#include "NetworkResponseParser.h"

using namespace Osp::Base;
using namespace Osp::Base::Collection;
using namespace Osp::Web::Json;

NetworkResponseParser::NetworkResponseParser()
{
}

NetworkResponseParser::~NetworkResponseParser()
{
}

result
NetworkResponseParser::Construct()
{
    return E_SUCCESS;
}

Osp::Base::Collection::IList *
NetworkResponseParser::PrepareDataN(RequestType t, shared_request_ptr req
    , result & errorCode)
{
	errorCode = req->GetErrorCode();
	if (errorCode != E_SUCCESS || req.get() == null) {
		AppLogDebug("%s", GetErrorMessage(errorCode));
		return null;
	}

	Osp::Base::Collection::IList * retval = null;
	shared_bbuffer_ptr response = req->GetResponseData();

//	// TODO : workaround for ChangeJobStatus request
//	if (t == REQUEST_TYPE_CHANGE_JOB_STATUS) {
//		ByteBuffer * buf = Osp::Base::Utility::StringUtil::StringToUtf8N(L"{\"value\":{\"status\":\"NOK\"}}");
//
//		response.reset(buf);
//	}

	if (response.get() == null) {
		errorCode = E_INVALID_DATA;
		return null;
	}

	Osp::Text::Latin1Encoding baseEnc;
	Osp::Text::Utf8Encoding destEnc;

	// change encoding before using as a utf-8 buffer
	shared_bbuffer_ptr respBuf(Osp::Text::Encoding::ConvertN(baseEnc, destEnc, *response));

	// TODO: it's a dirty workaround, because server adds to response ending strange code as in endingSample string
	int cap = respBuf->GetCapacity();

	String endingSample(L"{\"value\":{\"status\":\"NOK\"}}");
	String ending;
	destEnc.GetString(*respBuf, cap - endingSample.GetLength() - 1, endingSample.GetLength() + 1, ending);
	LOG_STRING(ending);

	if (t == REQUEST_TYPE_JOB_INFO && ending.Equals(endingSample) &&
			respBuf->GetCapacity() > endingSample.GetLength() + 1)
	{
		respBuf->SetByte(cap - endingSample.GetLength() - 1, 0);
	}

	SetLastResult(E_SUCCESS);
	boost::shared_ptr<JsonObject> obj(static_cast<JsonObject*>(JsonParser::ParseN(*respBuf)));
	if (obj.get() == null) {
		LOG_;
		errorCode = GetLastResult();
		AppLogDebug("Error code: %s", GetErrorMessage(errorCode));
		return null;
	}

	LOG_;
	switch (t)
	{
	case REQUEST_TYPE_VERSIONS_LIST :
	{
		JsonArray * array = GetFirstArray(obj.get(), VALUES);
		retval = GetPairsListN(array, ID, VERSION);
		if (retval != null) {
			LOG_INT(retval->GetCount());
		} else {
			LOG_;
		}
		break;
	}
	case REQUEST_TYPE_DISCIPLINES_LIST :
	case REQUEST_TYPE_LOCALES_LIST :
	case REQUEST_TYPE_SECTORS_LIST :
	{
		JsonArray * array = GetFirstArray(obj.get(), VALUES);
		retval = GetPairsListN(array, ID, NAME);
		break;
	}
	case REQUEST_TYPE_ALL_DATA_LISTS :
	{
		retval = GetAllDataListsN(obj.get());
		LOG_;
		break;
	}
	case REQUEST_TYPE_JOBS_IN_SEARCH :
	case REQUEST_TYPE_CREATE_SEARCH :
	{
		LOG_;
		retval = GetJobsListN(GetFirstArray(obj.get(), VALUES));
		break;
	}
	case REQUEST_TYPE_DELETE_SEARCH :
	{
		LOG_;
		JsonString * response = GetFirstString(obj.get(), DELETE);
		if (errorCode == E_SUCCESS && response != null &&
				response->Equals(String(RESPONSE_OK)))
		{
			errorCode = E_SUCCESS;
			ArrayList * list = new ArrayList();
			list->Construct();
			retval = list;

		} else {
			errorCode = E_FAILURE;
		}
		break;
	}
	case REQUEST_TYPE_BADGES_NUMBERS :
	{
		LOG_;
		JsonString * respStatus = GetFirstString(obj.get(), MYSEARCHES);
		if (respStatus != null && respStatus->Equals(String(RESPONSE_FAIL)))
		{
			errorCode = E_FAILURE;
			break;
		}

		JsonArray * respArr = GetFirstArray(obj.get(), MYSEARCHES);
		retval = GetBadgeNumbersList(respArr);
		break;
	}
	case REQUEST_TYPE_JOB_INFO : {
		LOG_;
		JsonObject * response = GetFirstObject(obj.get(), VALUE);
		if (errorCode == E_SUCCESS && response != null) {
			std::unique_ptr<Object> job(GetJobFromObjectN(response));

			LOG_;
			if (job.get() == null) {
				LOG_;
				errorCode = E_FAILURE;
			} else {
				ArrayList * list = new ArrayList();
				list->Construct();
				list->Add(*job.release());
				retval = list;
			}
		} else {
			LOG_;
			errorCode = E_FAILURE;
		}
		break;
	}
	case REQUEST_TYPE_CHANGE_JOB_STATUS : {
		if (errorCode != E_SUCCESS)
			break;

	    IMapEnumeratorT<const String*, IJsonValue*> * enumerator = obj->GetMapEnumeratorN();
	    RET_NULL_IF_FAIL(enumerator->Reset());

	    String reason;
	    int code;

	    while (enumerator->MoveNext() == E_SUCCESS)
	    {
	        const String *key;
	        IJsonValue* value;
	        RET_NULL_IF_FAIL(enumerator->GetKey(key));
	        RET_NULL_IF_FAIL(enumerator->GetValue(value));

	        LOG_STRING((*key));
	        if (key->Equals(String(CODE)) &&
	        		value->GetType() == JSON_TYPE_NUMBER)
	        {
	        	code = static_cast<JsonNumber*>(value)->ToInt();
	        } else if (key->Equals(String(REASON)) &&
	        		value->GetType() == JSON_TYPE_STRING)
	        {
	        	reason = *static_cast<JsonString*>(value);
	        }
	    }

	    delete enumerator;

		ArrayList * list = new ArrayList();
		list->Construct();
		SimpleJobItem * item = new SimpleJobItem;
		item->jobStatus = SimpleJobItem::JOB_STATUS_APPLIED;
		list->Add(*item);
		list->Add(*new Integer(code));
		list->Add(*new String(reason));
		retval = list;
		break;
	}
	case REQUEST_TYPE_SEND_MAIL_TO_REGISTER :
	{
		LOG_;
		JsonString * response = GetFirstString(obj.get(), RESPONSE_MAIL);
		if (response != null) LOG_STRING((*response));
		if (response == null || !response->Equals(String(RESPONSE_OK))) {
			LOG_;
			if (errorCode == E_SUCCESS) {
				errorCode = E_FAILURE;
			}
		}
		break;
	}
	case REQUEST_TYPE_SEND_MAIL_TO_RESTORE :
	{
		LOG_;
		JsonString * response = GetFirstString(obj.get(), RESPONSE_PASSWORD);
		if (response != null) LOG_STRING((*response));
		if (response == null || !response->Equals(String(RESPONSE_OK))) {
			LOG_;
			if (errorCode == E_SUCCESS) {
				errorCode = E_FAILURE;
			}
		}
		break;
	}
	case REQUEST_TYPE_SEND_LOGIN :
	{
		LOG_;
		JsonObject * userData = GetFirstObject(obj.get(), USER);
		retval = GetUserDataN(userData, errorCode);
		break;
	}
	case REQUEST_TYPE_SYNCHRONIZE_DATA :
	{
		JsonArray * response = GetFirstArray(obj.get(), JOBS);
		retval = GetIntegerArrayN(response);
		break;
	}
	default:
		break;
	}

	LOG_;
    return retval;
}

Osp::Web::Json::JsonString *
NetworkResponseParser::GetFirstString(
		Osp::Web::Json::JsonObject * head, const Osp::Base::String& waitKey)
{
    IMapEnumeratorT<const String*, IJsonValue*> * enumerator = head->GetMapEnumeratorN();
    RET_NULL_IF_FAIL(enumerator->Reset());

    JsonString* retval = null;

    while (enumerator->MoveNext() == E_SUCCESS)
    {
        const String *key;
        IJsonValue* value;
        RET_NULL_IF_FAIL(enumerator->GetKey(key));
        RET_NULL_IF_FAIL(enumerator->GetValue(value));

        LOG_STRING((*key));
        if (key->Equals(waitKey) && value->GetType() == JSON_TYPE_STRING) {
        	retval = static_cast<JsonString*>(value);
        }
    }

    delete enumerator;

    return retval;
}

Osp::Web::Json::JsonArray *
NetworkResponseParser::GetFirstArray(
		Osp::Web::Json::JsonObject * head, const Osp::Base::String& waitKey)
{
    IMapEnumeratorT<const String*, IJsonValue*> * enumerator = head->GetMapEnumeratorN();
    RET_NULL_IF_FAIL(enumerator->Reset());

    JsonArray* retval = null;

    while (enumerator->MoveNext() == E_SUCCESS)
    {
        const String *key;
        IJsonValue* value;
        RET_NULL_IF_FAIL(enumerator->GetKey(key));
        RET_NULL_IF_FAIL(enumerator->GetValue(value));

        if (key->Equals(waitKey) && value->GetType() == JSON_TYPE_ARRAY) {
        	retval = static_cast<JsonArray*>(value);
        	break;
        }
    }

    delete enumerator;

    return retval;
}

Osp::Web::Json::JsonObject *
NetworkResponseParser::GetFirstObject(
		Osp::Web::Json::JsonObject * head, const Osp::Base::String& waitKey)
{
    IMapEnumeratorT<const String*, IJsonValue*> * enumerator = head->GetMapEnumeratorN();
    RET_NULL_IF_FAIL(enumerator->Reset());

    JsonObject* retval = null;

    while (enumerator->MoveNext() == E_SUCCESS)
    {
        const String *key;
        IJsonValue* value;
        RET_NULL_IF_FAIL(enumerator->GetKey(key));
        RET_NULL_IF_FAIL(enumerator->GetValue(value));

        if (key->Equals(waitKey) && value->GetType() == JSON_TYPE_OBJECT) {
        	retval = static_cast<JsonObject*>(value);
        }
    }

    delete enumerator;

    return retval;
}

Osp::Base::Collection::IList *
NetworkResponseParser::GetPairsListN(JsonArray* array, const String& keyName,
		const String& keyValue)
{
	IList * retval = null;
	ArrayList * list = new ArrayList();
	list->Construct();

	IEnumeratorT<IJsonValue*> * enumerator = array->GetEnumeratorN();
	RET_NULL_IF_FAIL(enumerator->Reset());

	LOG_;
	while (enumerator->MoveNext() == E_SUCCESS)
	{
		IJsonValue * value;
		enumerator->GetCurrent(value);
		JsonObject* object = static_cast<JsonObject*>(value);

//		list->Add(*GetPairFromObjectN(object, keyName, keyValue));
		SortedAdd(list, GetStringPairFromObjectN(object, keyName, keyValue));
	}

	LOG_;
	delete enumerator;

	retval = list;
	return retval;
}

StringPair *
NetworkResponseParser::GetStringPairFromObjectN(Osp::Web::Json::JsonObject * obj,
		const Osp::Base::String& keyName, const Osp::Base::String& keyValue)
{
    IMapEnumeratorT<const String*, IJsonValue*> * enumerator = obj->GetMapEnumeratorN();
    RET_NULL_IF_FAIL(enumerator->Reset());

    StringPair * pair = new StringPair();

    while (enumerator->MoveNext() == E_SUCCESS)
    {
        const String *key;
        IJsonValue* value;
        RET_NULL_IF_FAIL(enumerator->GetKey(key));
        RET_NULL_IF_FAIL(enumerator->GetValue(value));

        if (key->Equals(keyName)) {
        	pair->first = *(static_cast<JsonString*>(value));
//    		LOG_STRING(pair->first);
        } else if (key->Equals(keyValue)) {
        	pair->second = *(static_cast<JsonString*>(value));
//    		LOG_STRING(pair->second);
        }
    }

    delete enumerator;

    return pair;
}

IntegerPair *
NetworkResponseParser::GetIntPairFromObjectN(Osp::Web::Json::JsonObject * obj,
		const Osp::Base::String& keyName, const Osp::Base::String& keyValue)
{
    IMapEnumeratorT<const String*, IJsonValue*> * enumerator = obj->GetMapEnumeratorN();
    RET_NULL_IF_NULL(enumerator);
    RET_NULL_IF_FAIL(enumerator->Reset());

    IntegerPair * pair = new IntegerPair();

    while (enumerator->MoveNext() == E_SUCCESS)
    {
        const String *key;
        IJsonValue* value;
        RET_NULL_IF_FAIL(enumerator->GetKey(key));
        RET_NULL_IF_FAIL(enumerator->GetValue(value));

        int val = 0;

        if (value->GetType() == JSON_TYPE_NUMBER) {
    		val = static_cast<JsonNumber*>(value)->ToInt();
    	} else if (value->GetType() == JSON_TYPE_STRING) {
    		Integer::Parse(*static_cast<JsonString*>(value), val);
    	}

        if (key->Equals(keyName)) {
        	pair->first = val;
        } else if (key->Equals(keyValue)) {
        	pair->second = val;
        }
    }

    delete enumerator;

    return pair;
}


Osp::Base::Collection::IList *
NetworkResponseParser::GetAllDataListsN(JsonObject * obj)
{
	LOG_;
	IList * retval = null;

	ArrayList * list = new ArrayList();
	list->Construct();

	LOG_;
	JsonArray * metiersArr = GetFirstArray(GetFirstObject(obj, VALUES), KEY_METIERS);
	JsonArray * secteursArr = GetFirstArray(GetFirstObject(obj, VALUES), KEY_SECTORS);
	JsonArray * localisztionsArr = GetFirstArray(GetFirstObject(obj, VALUES), KEY_LOCALES);
	LOG_;

	IList * list1 = GetPairsListN(metiersArr, ID, NAME);
	IList * list2 = GetPairsListN(secteursArr, ID, NAME);
	IList * list3 = GetPairsListN(localisztionsArr, ID, NAME);
	LOG_;


	list->AddItems(*list1);
	list->Add(*new StringPair);
	list->AddItems(*list2);
	list->Add(*new StringPair);
	list->AddItems(*list3);
	list->Add(*new StringPair);
	LOG_;

	list1->RemoveAll(false);
	list2->RemoveAll(false);
	list3->RemoveAll(false);
	delete list1;
	delete list2;
	delete list3;
	LOG_;

	retval = list;
	return retval;
}

Osp::Base::Collection::IList *
NetworkResponseParser::GetJobsListN(Osp::Web::Json::JsonArray * arr)
{
	Osp::Base::Collection::IList * retval = null;

	ArrayList * list = new ArrayList();
	list->Construct();

	IEnumeratorT<IJsonValue*> * enumerator = arr->GetEnumeratorN();
	RET_NULL_IF_FAIL(enumerator->Reset());

	LOG_;
	while (enumerator->MoveNext() == E_SUCCESS)
	{
		IJsonValue * value;
		enumerator->GetCurrent(value);
		JsonObject* object = static_cast<JsonObject*>(value);
		list->Add(*GetJobFromObjectN(object));
	}

	LOG_;
	delete enumerator;

	retval = list;

	return retval;
}

Osp::Base::Object *
NetworkResponseParser::GetJobFromObjectN(Osp::Web::Json::JsonObject * obj)
{
	LOG_;
	Osp::Base::Object * retval = null;

	IMapEnumeratorT<const String*, IJsonValue*> * enumerator = obj->GetMapEnumeratorN();
    RET_NULL_IF_FAIL(enumerator->Reset());

    std::unique_ptr<FullJobItem> item(new FullJobItem);

    while (enumerator->MoveNext() == E_SUCCESS)
    {
        const String *key;
        IJsonValue* value;
        RET_NULL_IF_FAIL(enumerator->GetKey(key));
        RET_NULL_IF_FAIL(enumerator->GetValue(value));

        if (key->Equals(String(RESPONSE_STATUS))) {
        	if (static_cast<JsonString*>(value)->Equals(String(RESPONSE_FAIL))) {
        		LOG_;
        		return null;
        	}
        } else if (key->Equals(String(JOB_REFERENCE))) {
        	item->reference = *static_cast<JsonString*>(value);
        	item->jobId = static_cast<JsonNumber*>(value)->ToInt();
        	LOG_STRING((*key));
        	LOG_INT((static_cast<JsonNumber*>(value)->ToInt()));
        } else if (key->Equals(String(JOB_CONSULTANT))) {
        	item->consultant = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_CONSULTANT_REF))) {
        	item->consultantReference = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_CONSULTANT_NAME))) {
        	item->consultantName = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_TITLE))) {
        	item->title = *static_cast<JsonString*>(value);
        	item->jobName = item->title;
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_OFFER_URL))) {
        	item->offerURL = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_METIER_ID))) {
        	item->idMetier = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_METIER))) {
        	item->metier = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_SECTEUR_ID))) {
        	item->idSecteur = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_SECTEUR))) {
        	item->secteur = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_LOCALE_ID))) {
        	item->idLocalisation = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_LOCALE))) {
        	item->localisation = *static_cast<JsonString*>(value);
        	item->jobLocation = item->localisation;
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_LOCALE_TXT))) {
        	item->txtLocalisation = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_CONTRAT))) {
        	item->contrat = *static_cast<JsonString*>(value);
        	item->jobContract = item->contrat;
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_OFFER_DURATION))) {
        	item->offerDuration = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_DISPLAY_SALARY))) {
        	item->displaySalary = static_cast<JsonBool*>(value)->ToBool();
        	LOG_STRING((*key));
        	LOG_BOOL((static_cast<JsonBool*>(value)->ToBool()));
        } else if (key->Equals(String(JOB_SALARY_MAX))) {
        	item->highSalary = static_cast<JsonNumber*>(value)->ToInt();
        	LOG_STRING((*key));
        	LOG_INT((static_cast<JsonNumber*>(value)->ToInt()));
        } else if (key->Equals(String(JOB_SALARY_MIN))) {
        	item->lowSalary = static_cast<JsonNumber*>(value)->ToInt();
        	LOG_STRING((*key));
        	LOG_INT((static_cast<JsonNumber*>(value)->ToInt()));
        } else if (key->Equals(String(JOB_DESCRIPTION))) {
        	item->description = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_REQUIREMENTS))) {
        	item->requirements = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_CLIENT_DESCRIPTION))) {
        	item->clientDescription = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_BENEFITS))) {
        	item->benefits = *static_cast<JsonString*>(value);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        } else if (key->Equals(String(JOB_DATE))) {
        	item->date = *static_cast<JsonString*>(value);
        	GetDateFromString(item->jobDate, item->date);
        	LOG_STRING((*key));
        	LOG_STRING((*static_cast<JsonString*>(value)));
        }

    }

    delete enumerator;

    if (item->displaySalary) {
    	if (item->lowSalary > 0 && item->highSalary >= item->lowSalary) {
    		item->jobSalary = GetFormattedPrice(item->lowSalary) + Strings::euro +
    		        L" - " + GetFormattedPrice(item->highSalary) + Strings::euro;
    	} else if (item->highSalary > 0) {
    		item->jobSalary = GetFormattedPrice(item->highSalary) + Strings::euro;
    	} else if (item->lowSalary > 0) {
    		item->jobSalary = GetFormattedPrice(item->lowSalary) + Strings::euro;
    	}
    }

    retval = item.release();
	return retval;
}

Osp::Base::Collection::IList *
NetworkResponseParser::GetUserDataN(Osp::Web::Json::JsonObject * obj,
		result & errorCode)
{
	Osp::Base::Collection::IList * retval = null;

	if (obj == null) return null;

	IMapEnumeratorT<const String*, IJsonValue*> * enumerator = obj->GetMapEnumeratorN();
    RET_NULL_IF_FAIL(enumerator->Reset());

    UserData * item = new UserData;

    while (enumerator->MoveNext() == E_SUCCESS)
    {
        const String *key;
        IJsonValue* value;
        RET_NULL_IF_FAIL(enumerator->GetKey(key));
        RET_NULL_IF_FAIL(enumerator->GetValue(value));

        if (key->Equals(String(USER_ACCOUNT))) {
        	item->userAccount = *static_cast<JsonString*>(value);
        } else if (key->Equals(String(USER_PHONE))) {
        	item->userPhone = *static_cast<JsonString*>(value);
        } else if (key->Equals(String(USER_ID))) {
        	item->userId = *static_cast<JsonString*>(value);
        } else if (key->Equals(String(USER_CV))) {
        	item->userCv = *static_cast<JsonString*>(value);
        } else if (key->Equals(String(USER_NAME))) {
        	item->userName = *static_cast<JsonString*>(value);
        } else if (key->Equals(String(USER_LASTNAME))) {
        	item->userLastName = *static_cast<JsonString*>(value);
        }
    }


    delete enumerator;

    if (item->userAccount.Equals(String(L"0")) || item->userId.Equals(String(L"0")))
    {
    	LOG_;
    	errorCode = E_AUTHENTICATION;
    	delete item;
    } else {
    	LOG_;
    	ArrayList * list = new ArrayList();
    	list->Construct();
    	list->Add(*item);
    	retval = list;
    }
	return retval;
}

result
NetworkResponseParser::SortedAdd(IList * list, StringPair * pair)
{
	if (list == null || pair == null) {
		return E_INVALID_ARG;
	}

	int beginPos = 0;
	int endPos = list->GetCount();

	int cnt = 0;

	// logarithmic sorted insertion

	while (beginPos < endPos) {
		int newPos = (endPos + beginPos) / 2;

		StringPair * atNewPos = static_cast<StringPair*>(list->GetAt(newPos));

		if (Osp::Base::String::Compare(pair->second, atNewPos->second) < 0)
		{
			endPos = newPos;
		} else {
			beginPos = newPos + 1;
		}
		cnt ++;
	}

	LOG_STRING(pair->second);
	LOG_INT(endPos);

	list->InsertAt(*pair, endPos);
//	list->Add(*pair);
	return E_SUCCESS;
}

Osp::Base::Collection::IList *
NetworkResponseParser::GetBadgeNumbersList(JsonArray* arr)
{
	Osp::Base::Collection::IList * retval = null;

	ArrayList * list = new ArrayList();
	list->Construct();

	IEnumeratorT<IJsonValue*> * enumerator = arr->GetEnumeratorN();
	RET_NULL_IF_FAIL(enumerator->Reset());

	LOG_;
	while (enumerator->MoveNext() == E_SUCCESS)
	{
		LOG_;
		IJsonValue * value;
		enumerator->GetCurrent(value);
		JsonObject* object = static_cast<JsonObject*>(value);
		list->Add(*GetIntPairFromObjectN(object, String(MYSEARCHES_SID),
				String(MYSEARCHES_NB)));
	}

//	for (int i = 0; i < list->GetCount(); i ++) {
//	    static_cast<IntegerPair*>(list->GetAt(i))->second = Integer(1);
//	}

	LOG_;
	delete enumerator;

	retval = list;


	return retval;
}

Osp::Base::Collection::IList *
NetworkResponseParser::GetIntegerArrayN(Osp::Web::Json::JsonArray * arr)
{
	Osp::Base::Collection::IList * retval = null;

	ArrayList * list = new ArrayList();
	list->Construct();

	IEnumeratorT<IJsonValue*> * enumerator = arr->GetEnumeratorN();
	RET_NULL_IF_FAIL(enumerator->Reset());

	while (enumerator->MoveNext() == E_SUCCESS)
	{
		IJsonValue * value;
		enumerator->GetCurrent(value);
		JsonNumber * num = static_cast<JsonNumber*>(value);
		list->Add(*new Integer(num->ToInt()));
	}

	delete enumerator;
	retval = list;

	return retval;
}
