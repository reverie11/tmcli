_tmcli_completion() {
    local cur prev words cword
    _get_comp_words_by_ref -n : cur prev words cword

    local array=("${words[@]:1:cword-1}")

    local raw=$(tmcli --complete-args "${array[@]}")
    eval "local completions=($raw)"

    local IFS=$'\n'
    COMPREPLY=($(compgen -W "${completions[*]}" -- "$cur"))
    __ltrim_colon_completions "$cur"
}

complete -F _tmcli_completion tmcli
