from datetime import datetime
from pydantic import BaseModel, Field, ValidationError
from typing import Any, Optional

class IpcMessageException(Exception):
    ...

class IpcMessage(BaseModel):

    id: Optional[int] = None
    msg_type: Optional[str] = None
    msg_val: Optional[str] = None
    timestamp: datetime = Field(default_factory=datetime.now)
    params: dict[str, Any] = Field(default_factory=dict)

    def __init__(self, *args, from_str: str = None, **kwargs):
        if from_str is not None:
            obj = self.__class__.from_str(from_str)
            super().__init__(**obj.model_dump())
        else:
            super().__init__(*args, **kwargs)

    @classmethod
    def from_str(cls, json_str)  -> "IpcMessage":
        try:
            return cls.model_validate_json(json_str)
        except ValidationError as e:
            raise IpcMessageException("Illegal message JSON format: " + str(e)) from e

    def encode(self):
        return self.model_dump_json()

    def is_valid(self) -> bool:
        required_fields = ['id', 'msg_type', 'msg_val', 'timestamp']
        return all(getattr(self, field, None) is not None for field in required_fields)

    def get_msg_type(self) -> str:
        return self.msg_type

    def get_msg_val(self) -> str:
        return self.msg_val

    def get_msg_timestamp(self) -> str:
        return self.timestamp.isoformat()

    def get_msg_id(self) -> int:
        return self.id

    def get_param(self, param_name: str, default_value: Any = None) -> Any:
        try:
            return self.params[param_name]
        except KeyError:
            if default_value is None:
                raise IpcMessageException(f"Missing parameter {param_name}")
            return default_value

    def get_params(self) -> dict[str, Any]:
        return self.params

    def set_msg_type(self, msg_type: str) -> None:
        self.msg_type = msg_type

    def set_msg_val(self, msg_val: str) -> None:
        self.msg_val = msg_val

    def set_msg_id(self, msg_id: int) -> None:
        self.id = msg_id

    def set_param(self, param_name: str, param_value: Any) -> None:
        self.params[param_name] = param_value

    def set_params(self, params: dict[str, Any]) -> None:
        self.params.update(params)
